#define _DEFAULT_SOURCE
#include <ctype.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "db.h"
#include "facebooc_server.h"
#include "http/cookies.h"
#include "models/account.h"
#include "models/connection.h"
#include "models/like.h"
#include "models/post.h"
#include "models/session.h"
#include "server.h"
#include "template.h"
#include "utility.h"

// Facebooc is an directive class of Server
struct Facebooc {
    Server *server;
};

struct Buf_size {
    void *addr;
    size_t len;
};

static Response *home(Request *);
static Response *dashboard(Request *);
static Response *profile(Request *);
static Response *post(Request *);
static Response *like(Request *);
static Response *unlike(Request *);
static Response *connect(Request *);
static Response *search(Request *);
static Response *login(Request *);
static Response *logout(Request *);
static Response *signup(Request *);
static Response *static_handler(Request *req);
static Response *notFound(Request *);  // default route

static inline int get_id(const char *uri);
static const Account *get_account(const Cookies *c);

Facebooc *FB_new(const uint16_t port)
{
    Facebooc *s = malloc(sizeof(Facebooc));
    s->server = serverNew(port);
    serverAddHandler(s->server, "signup", signup);
    serverAddHandler(s->server, "logout", logout);
    serverAddHandler(s->server, "login", login);
    serverAddHandler(s->server, "search", search);
    serverAddHandler(s->server, "connect", connect);
    serverAddHandler(s->server, "like", like);
    serverAddHandler(s->server, "unlike", unlike);
    serverAddHandler(s->server, "post", post);
    serverAddHandler(s->server, "profile", profile);
    serverAddHandler(s->server, "dashboard", dashboard);
    serverAddHandler(s->server, "", home);
    serverAddHandler(s->server, "static", static_handler);

    set_callback(s->server, notFound);

    initDB();

    return s;
}

int FB_run(Facebooc *s)
{
    serverServe(s->server);
    return 1;
}

void FB_delete(Facebooc *s)
{
    if (s->server)
        serverDel(s->server);
    void db_close();
    free(s);
    s = NULL;
}

static inline int invalid(Template *template,
                          const char *key,
                          const char *value)
{
    templateSet(template, key, value);
    return false;
}

// compare str1 and str2 from tail
static inline int rev_cmp(const char *str1,
                          size_t len1,
                          const char *str2,
                          size_t len2)
{
    const long smaller = min(len1, len2);
    return strncmp(str1 + len1 - smaller, str2 + len2 - smaller, smaller);
}

static inline const char *mimeType_mux(const char *path)
{
    static const char *file_type[] = {
        "html", "json", "jpeg", "jpg", "gif", "png", "css", "js",
    };
    static const size_t file_type_len = ARR_LEN(file_type);

    static const char *mime_type[] = {
        "text/html",  "application/json",
        "image/jpeg", "image/jpeg",
        "image/gif",  "image/png",
        "text/css",   "application/javascript",
        "text/plain",
    };

    size_t i = 0;
    for (; i < file_type_len; i++) {
        if (rev_cmp(path, strlen(path), file_type[i], strlen(file_type[i])) ==
            0)
            break;
    }

    return mime_type[i];
}

// Each entry of this Hash_map is referenced by mmap
// Don't use free to munmap it
// Also, not expected to munmap it
static Hash_map *static_file_table;

static struct Buf_size get_static_file(const char *filename)
{
    struct Buf_size fm = {.addr = NULL, .len = 0};
    // Check if it is already in the static_file_table
    const struct Buf_size *entry = Hash_map_get(static_file_table, filename);
    if (entry) {  // Found
        fm.addr = entry->addr;
        fm.len = entry->len;
        goto found;
    }

    // EXIT ON DIRS
    struct stat stat_file;
    if (stat(filename, &stat_file) < 0 || S_ISDIR(stat_file.st_mode))
        goto fail;

    // EXIT ON NOT FOUND
    int file = open(filename, O_RDONLY);
    if (unlikely(file == -1)) {
        // Not found or error
        fprintf(stderr, "%s not found\n", filename);
        goto fail;
    };

    // Init local fm
    fm.addr = mmap(NULL, stat_file.st_size, PROT_READ, MAP_PRIVATE, file, 0);
    fm.len = stat_file.st_size;
    madvise(fm.addr, stat_file.st_size,
            MADV_SEQUENTIAL | MADV_WILLNEED | MADV_DONTFORK);
    close(file);

    // Dup a fm into static_file_table
    struct Buf_size *copy_fm = malloc(sizeof(struct Buf_size));
    copy_fm->addr = fm.addr;
    copy_fm->len = fm.len;
    SPair p = {.key = (char *) filename, .value = copy_fm};
    Hash_map_insert_move(static_file_table, &p);

fail:
found:
    return fm;
}

static Response *static_handler(Request *req)
{
    // EXIT ON SHENANIGANS
    if (strstr(req->uri, "../"))
        return NULL;

    const char *filename = req->uri + 1;

    // Prepare static_file_table
    if (unlikely(!static_file_table))
        static_file_table = Hash_map_new();

    struct Buf_size fm = get_static_file(filename);
    if (fm.addr == NULL)  // file not found
        return NULL;

    Response *response = responseNew();
    responseSetBody(
        response,
        (Basic_string *) &fm);  // fm's memory model is same as Basic_string

    // MIME TYPE
    // It is impossible to not found any matched mimetype, because we had check
    // static file is exist
    const char *mimeType = mimeType_mux(req->uri);
    char content_len[11];
    snprintf(content_len, 11, "%ld", fm.len);

    // RESPOND
    responseSetStatus(response, OK);
    responseAddHeader(response,
                      eXpire_pair("Content-Type", (char *) mimeType, SSPair));
    responseAddHeader(response,
                      eXpire_pair("Content-Length", content_len, SSPair));
    responseAddHeader(response,
                      eXpire_pair("Cache-Control", "max-age=2592000", SSPair));
    return response;
}

// Get sid string from cookies
static const Account *get_account(const Cookies *c)
{
    // TODO: Use an object pool to reduce malloc times
    if (unlikely(c == NULL))
        return NULL;
    Cookie *cookie = Cookies_get(c, "sid");
    const Basic_string sid = {.data = (char *) Cookie_get_attr(cookie, VALUE),
                              .size = strlen(Cookie_get_attr(cookie, VALUE))};
    return accountGetBySId(get_db(), &sid);
}

static Response *home(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (my_acc) {
        accountDel((Account *) my_acc);
        return responseNewRedirect("/dashboard/");
    }

    Response *response = responseNew();
    Template *template = templateNew("templates/index.html");
    responseSetStatus(response, OK);
    templateSet(template, "active", "home");
    templateSet(template, "subtitle", "Home");
    responseSetBody_move(response, templateRender(template));
    templateDel(template);
    return response;
}

static Response *dashboard(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(my_acc == NULL))
        return NULL;

    Posts *posts = postGetLatestGraph(get_db(), my_acc->id, 0);

    // A flexible string containing the returning posts' context
    Basic_string *ctx = NULL;
    if (!Posts_is_empty(posts))
        ctx = Basic_string_init("<ul class=\"posts\">");

    List *cur = NULL;
    list_for_each(cur, &posts->list)
    {
        Post *p = container_of(cur, Posts, list)->p;

        // Post's author name is the only data we need
        Account *post_author = accountGetById(get_db(), p->authorId);
        Basic_string *author_name = post_author->name;
        post_author->name = NULL;
        accountDel(post_author);

        // Generate a post entry
        size_t entry_size =
            p->body->size + author_name->size + 86;  // 86 is a fmt str
        char *entry_buf = malloc(entry_size + 1);
        snprintf(entry_buf, entry_size,
                 "<li class=\"post-item\">"
                 "<div class=\"post-author\">%s</div>"
                 "<div class=\"post-content\">"
                 "%s"
                 "</div>",
                 author_name->data, p->body->data);
        Basic_string_delete(author_name);
        // Append post entry into returning ctx
        Basic_string_append_raw(ctx, entry_buf);
        free(entry_buf);

        // Add like/unlike button
        char sbuff[128];
        bool liked = likeLiked(get_db(), my_acc->id, p->id);
        if (liked)  // If I liked it
            snprintf(sbuff, 55,
                     "<a class=\"btn\" href=\"/unlike/%d/\">Liked</a> - ",
                     p->id);
        else
            snprintf(sbuff, 52,
                     "<a class=\"btn\" href=\"/like/%d/\">Like</a> - ", p->id);
        Basic_string_append_raw(ctx, sbuff);

        // Add post's create time
        time_t t = p->createdAt;
        struct tm *info = gmtime(&t);
        info->tm_hour = info->tm_hour + 8;
        strftime(sbuff, 128, "%c GMT+8", info);

        Basic_string_append_raw(ctx, sbuff);
        Basic_string_append_raw(ctx, "</li>");
    }
    Posts_delete(posts);

    Template *template = templateNew("templates/dashboard.html");
    if (ctx) {
        Basic_string_append_raw(ctx, "</ul>");
        templateSet(template, "graph", ctx->data);
    } else {
        templateSet(template, "graph",
                    "<ul class=\"posts\"><div class=\"not-found\">Nothing "
                    "here.</div></ul>");
    }
    Basic_string_delete(ctx);

    templateSet(template, "active", "dashboard");
    templateSet(template, "loggedIn", "t");
    templateSet(template, "subtitle", "Dashboard");
    templateSet(template, "accountName", my_acc->name->data);

    Response *response = responseNew();
    responseSetStatus(response, OK);
    responseSetBody_move(response, templateRender(template));
    templateDel(template);
    accountDel((Account *) my_acc);
    return response;
}

// Preview someone's profile page
static Response *profile(Request *req)
{
    // Check I'm logged-in
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(!my_acc))
        return NULL;

    const Account *acc2 = accountGetById(get_db(), get_id(req->uri));

    if (!acc2) {
        accountDel((Account *) my_acc);
        return NULL;
    }
    if (acc2->id == my_acc->id) {
        accountDel((Account *) my_acc);
        accountDel((Account *) acc2);
        return responseNewRedirect("/dashboard/");
    }

    // Max length of uid is len(INT_MAX), in other words, the maxlen of uid is
    // 10
    char acc2_id_str[11];
    snprintf(acc2_id_str, 10, "%d", acc2->id);

    Connection *connection =
        connectionGetByAccountIds(get_db(), my_acc->id, acc2->id);
    char connectStr[128];  // Name max len is 50
    if (connection) {
        snprintf(connectStr, 26 + acc2->name->size, "You and %s are connected!",
                 acc2->name->data);
    } else {
        snprintf(connectStr, 76 + acc2->name->size + 10,
                 "You and %s are not connected."
                 " <a href=\"/connect/%d/\">Click here</a> to connect!",
                 acc2->name->data, acc2->id);
    }
    connectionDel(connection);

    Posts *posts = postGetLatest(get_db(), acc2->id, 0);

    Basic_string *ctx = NULL;
    if (!Posts_is_empty(posts))
        ctx = Basic_string_init("<ul class=\"posts\">");

    List *cur = NULL;
    list_for_each(cur, &posts->list)
    {
        Post *p = container_of(cur, Posts, list)->p;

        // Post's entry
        char *entry_buf = malloc(p->body->size + 55);
        snprintf(entry_buf, 54 + p->body->size,
                 "<li class=\"post-item\"><div class=\"post-author\">%s</div>",
                 p->body->data);
        Basic_string_append_raw(ctx, entry_buf);
        free(entry_buf);

        // Append liked entry
        char sbuff[128];
        bool liked = likeLiked(get_db(), my_acc->id, p->id);
        if (liked)
            Basic_string_append_raw(ctx, "Liked - ");
        else {
            snprintf(sbuff, 52,
                     "<a class=\"btn\" href=\"/like/%d/\">Like</a> - ", p->id);
            Basic_string_append_raw(ctx, sbuff);
        }

        // Add post's create time
        time_t t__ = p->createdAt;
        strftime(sbuff, 128, "%c GMT", gmtime(&t__));
        Basic_string_append_raw(ctx, sbuff);
        Basic_string_append_raw(ctx, "</li>");
    }
    Posts_delete(posts);

    Template *template = templateNew("templates/profile.html");
    if (ctx) {
        Basic_string_append_raw(ctx, "</ul>");
        templateSet(template, "profilePosts", ctx->data);
    } else {
        templateSet(template, "profilePosts",
                    "<h4 class=\"not-found\">This person has not posted "
                    "anything yet!</h4>");
    }
    Basic_string_delete(ctx);

    templateSet(template, "active", "profile");
    templateSet(template, "loggedIn", "t");
    templateSet(template, "subtitle", acc2->name->data);
    templateSet(template, "profileId", acc2_id_str);
    templateSet(template, "profileName", acc2->name->data);
    templateSet(template, "profileEmail", acc2->email->data);
    templateSet(template, "profileConnect", connectStr);
    templateSet(template, "accountName", my_acc->name->data);

    Response *response = responseNew();
    responseSetStatus(response, OK);
    responseSetBody_move(response, templateRender(template));
    accountDel((Account *) my_acc);
    accountDel((Account *) acc2);
    templateDel(template);
    return response;
}

// I posted a post via HTTP POST
static Response *post(Request *req)
{
    const Account *acc = get_account(req->cookies);
    if (unlikely(acc == NULL))
        goto fail;

    if (unlikely(req->method != POST))
        goto fail;

    Basic_string *postStr = Basic_string_init(body_get(req->postBody, "post"));

    if (postStr->size == 0) {
        Basic_string_delete(postStr);
        goto fail;
    } else if (postStr->size < MAX_BODY_LEN)
        postCreate_move(get_db(), acc->id, postStr);

fail:
    accountDel((Account *) acc);
    return responseNewRedirect("/dashboard/");
}

static Response *unlike(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    char redir_to[32] = {"/dashboard/"};
    if (unlikely(my_acc == NULL))
        goto fail;

    const Post *post = postGetById(get_db(), get_id(req->uri));
    if (!post)
        goto fail;

    likeDel(likeDelete(get_db(), my_acc->id, post->authorId, post->id));

    if (query_get(req->queryString, "r"))
        snprintf(redir_to, 32, "/profile/%d/", post->authorId);

    postDel((Post *) post);

fail:
    accountDel((Account *) my_acc);
    return responseNewRedirect(redir_to);
}

static Response *like(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    char redir_to[32] = {"/dashboard/"};
    if (unlikely(my_acc == NULL))
        goto fail;

    const Post *post = postGetById(get_db(), get_id(req->uri));
    if (!post)
        goto fail;

    likeDel(likeCreate(get_db(), my_acc->id, post->authorId, post->id));

    if (query_get(req->queryString, "r"))
        snprintf(redir_to, 32, "/profile/%d/", post->authorId);

    postDel((Post *) post);

fail:
    accountDel((Account *) my_acc);
    return responseNewRedirect(redir_to);
}

static Response *connect(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    char redir_to[32] = {"/dashboard/"};
    if (unlikely(my_acc == NULL))
        goto fail;

    const Account *account = accountGetById(get_db(), get_id(req->uri));
    if (!account)
        goto fail;

    connectionDel(connectionCreate(get_db(), my_acc->id, account->id));

    snprintf(redir_to, 32, "/profile/%d/", account->id);

    accountDel((Account *) account);

fail:
    accountDel((Account *) my_acc);
    return responseNewRedirect(redir_to);
}

static Response *search(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(my_acc == NULL))
        return responseNewRedirect("/login/");
    accountDel((Account *) my_acc);

    const Basic_string *query =
        Basic_string_init(query_get(req->queryString, "q"));

    if (query->size == 0) {
        Basic_string_delete((Basic_string *) query);
        return NULL;
    }

    Basic_string *ctx = NULL;

    Accounts *as = accountSearch(get_db(), query, 0);
    if (!accounts_is_empty(as))
        ctx = Basic_string_init("<ul class=\"search-results\">");

    List *cur = NULL;
    list_for_each(cur, &as->list)
    {
        Account *a = container_of(cur, Accounts, list)->a;

        char sbuff[128];  // name and email's max len are both 50
        snprintf(sbuff, 62 + a->name->size + a->email->size,
                 "<li><a href=\"/profile/%d/\">%s</a> (<span>%s</span>)</li>\n",
                 a->id, a->name->data, a->email->data);
        Basic_string_append_raw(ctx, sbuff);
    }
    accounts_delete(as);

    if (ctx)
        Basic_string_append_raw(ctx, "</ul>");

    Template *template = templateNew("templates/search.html");

    if (!ctx)
        templateSet(template, "results",
                    "<h4 class=\"not-found\">There were no results "
                    "for your query.</h4>");
    else
        templateSet(template, "results", ctx->data);
    Basic_string_delete(ctx);

    templateSet(template, "searchQuery", query->data);
    templateSet(template, "active", "search");
    templateSet(template, "loggedIn", "t");
    templateSet(template, "subtitle", "Search");
    Basic_string_delete((Basic_string *) query);

    Response *response = responseNew();
    responseSetStatus(response, OK);
    responseSetBody_move(response, templateRender(template));
    templateDel(template);
    return response;
}

static Response *login(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(my_acc != NULL)) {  // It's not usually logined
        accountDel((Account *) my_acc);
        return responseNewRedirect("/dashboard/");
    }

    Response *response = responseNew();
    Template *template = templateNew("templates/login.html");
    responseSetStatus(response, OK);
    templateSet(template, "active", "login");
    templateSet(template, "subtitle", "Login");

    if (req->method == POST) {
        const Basic_string *username =
            Basic_string_init(body_get(req->postBody, "username"));
        const Basic_string *password =
            Basic_string_init(body_get(req->postBody, "password"));

        if (!username->size) {
            invalid(template, "usernameError", "Username is missing!");
        } else {
            templateSet(template, "formUsername", username->data);
        }

        if (!password->size) {
            invalid(template, "passwordError", "Password is missing!");
        }

        bool valid = account_auth(get_db(), username, password);

        if (valid) {
            Session *session =
                sessionCreate(get_db(), username->data, password->data);
            // Username and password are no longer to use
            Basic_string_delete((Basic_string *) username);
            Basic_string_delete((Basic_string *) password);

            if (session) {
                char expire_string[64];
                Cookie_gen_expire(expire_string, 3600 * 24 * 30);

                // Setting cookie
                Cookie *cookie =
                    Cookie_init(eXpire_pair("sid", session->sessionId, SSPair));
                Cookie_set_attr(cookie, EXPIRE, expire_string);
                responseAddCookie(response, cookie);

                sessionDel(session);
                Cookie_delete(cookie);

                responseSetStatus(response, FOUND);
                responseAddHeader(
                    response, eXpire_pair("Location", "/dashboard/", SSPair));
                goto ret;
            } else {
                invalid(template, "usernameError",
                        "Invalid username or password.");
            }
        } else {
            invalid(template, "usernameError", "Invalid username or password.");
        }

        Basic_string_delete((Basic_string *) username);
        Basic_string_delete((Basic_string *) password);
    }

    responseSetBody_move(response, templateRender(template));

ret:
    templateDel(template);
    return response;
}

static Response *logout(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(!my_acc))  // It's usually logged in
        return responseNewRedirect("/");

    accountDel((Account *) my_acc);

    Response *response = responseNewRedirect("/");

    Cookie *cookie = Cookie_init(eXpire_pair("sid", "", SSPair));
    char buf[64];
    Cookie_gen_expire(buf, -1);
    Cookie_set_attr(cookie, EXPIRE, buf);
    responseAddCookie(response, cookie);  // Copy
    Cookie_delete(cookie);                // delete

    return response;
}

static Response *signup(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(my_acc != NULL)) {
        // If you already logged in, you should not register a new account
        accountDel((Account *) my_acc);
        return responseNewRedirect("/dashboard/");
    }

    Response *response = responseNew();
    Template *template = templateNew("templates/signup.html");
    templateSet(template, "active", "signup");
    templateSet(template, "subtitle", "Sign Up");
    responseSetStatus(response, OK);

    if (req->method == POST) {
        bool valid = false;
        const Basic_string *name =
            Basic_string_init(body_get(req->postBody, "name"));
        const Basic_string *email =
            Basic_string_init(body_get(req->postBody, "email"));
        const Basic_string *username =
            Basic_string_init(body_get(req->postBody, "username"));
        const Basic_string *password =
            Basic_string_init(body_get(req->postBody, "password"));

        // Although confirmPassword is not used in register model, we also use
        // Basic_string for consistency
        const Basic_string *confirmPassword =
            Basic_string_init(body_get(req->postBody, "confirm-password"));

        if (!name->size) {
            invalid(template, "nameError", "You must enter your name!");
        } else if (name->size < 5 || name->size > 50) {
            invalid(template, "nameError",
                    "Your name must be between 5 and 50 characters long.");
        } else {
            templateSet(template, "formName", name->data);
            valid = true;
        }

        if (!email->size) {
            invalid(template, "emailError", "You must enter an email!");
        } else if (strchr(email->data, '@') == NULL) {
            invalid(template, "emailError", "Invalid email.");
        } else if (email->size < 3 || email->size > 50) {
            invalid(template, "emailError",
                    "Your email must be between 3 and 50 characters long.");
        } else if (!accountCheckEmail(get_db(), email)) {
            invalid(template, "emailError", "This email is taken.");
        } else {
            templateSet(template, "formEmail", email->data);
            valid = true;
        }

        if (!username->size) {
            invalid(template, "usernameError", "You must enter a username!");
        } else if (username->size < 3 || username->size > 50) {
            invalid(template, "usernameError",
                    "Your username must be between 3 and 50 characters long.");
        } else if (!accountCheckUsername(get_db(), username)) {
            invalid(template, "usernameError", "This username is taken.");
        } else {
            templateSet(template, "formUsername", username->data);
            valid = true;
        }

        if (!password->size) {
            invalid(template, "passwordError", "You must enter a password!");
        } else if (password->size < 8) {
            invalid(template, "passwordError",
                    "Your password must be at least 8 characters long!");
        } else
            valid = true;

        if (!confirmPassword->size) {
            invalid(template, "confirmPasswordError",
                    "You must confirm your password.");
        } else if (strcmp(password->data, confirmPassword->data) != 0) {
            invalid(template, "confirmPasswordError",
                    "The two passwords must be the same.");
        } else
            valid = true;

        if (valid) {
            // Register an account
            accountDel(
                accountCreate(get_db(), name, email, username, password));
            Basic_string_delete((Basic_string *) name);
            Basic_string_delete((Basic_string *) email);
            Basic_string_delete((Basic_string *) username);
            Basic_string_delete((Basic_string *) password);
            Basic_string_delete((Basic_string *) confirmPassword);

            responseSetStatus(response, FOUND);
            responseAddHeader(response,
                              eXpire_pair("Location", "/login/", SSPair));
            goto ret;
        }
    }

    responseSetBody_move(response, templateRender(template));
ret:
    templateDel(template);
    return response;
}

static Response *notFound(Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    const int is_loggedIn = !!my_acc;
    accountDel((Account *) my_acc);

    Template *template = templateNew("templates/404.html");
    templateSet(template, "loggedIn", is_loggedIn ? "t" : "");
    templateSet(template, "subtitle", "404 Not Found");

    Response *response = responseNew();
    responseSetStatus(response, NOT_FOUND);
    responseSetBody_move(response, templateRender(template));
    templateDel(template);
    return response;
}

static inline int get_id(const char *uri)
{
    // format: "/%s/<id>/"

    char *begin = strchr(uri + 1, '/') + 1;
    char *end = strchr(begin, '/');
    const size_t len = end - begin;

    char new_str[10] = {0};
    memcpy(new_str, begin, len);

    return atoi(new_str);
}
