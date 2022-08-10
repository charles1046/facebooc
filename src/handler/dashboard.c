#include "dashboard.h"
#include "db.h"
#include "helper.h"
#include "template.h"

#include "models/like.h"
#include "models/post.h"

#include <stdio.h>
#include <time.h>

static inline Basic_string *concat_posts(const Posts *ps,
                                         const Account *my_acc);
static inline Template *gen_template(const Basic_string *ctx,
                                     const Account *my_acc);

Response *dashboard(const Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(my_acc == NULL))
        return NULL;

    Posts *posts = postGetLatestGraph(get_db(), my_acc->id, 0);
    const Basic_string *ctx = concat_posts(posts, my_acc);
    Posts_delete(posts);

    Response *response = responseNew();
    responseSetStatus(response, OK);

    Template *template = gen_template(ctx, my_acc);
    responseSetBody_move(response, templateRender(template));
    templateDel(template);

    Basic_string_delete(ctx);
    accountDel((Account *) my_acc);
    return response;
}

static inline Basic_string *concat_posts(const Posts *posts,
                                         const Account *my_acc)
{
    if (Posts_is_empty(posts))
        return NULL;

    // A flexible string containing the returning posts' context
    Basic_string *ctx = Basic_string_init("<ul class=\"posts\">");

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
    Basic_string_append_raw(ctx, "</ul>");

    return ctx;
}

static inline Template *gen_template(const Basic_string *ctx,
                                     const Account *my_acc)
{
    Template *template = templateNew("templates/dashboard.html");
    if (ctx) {
        templateSet(template, "graph", ctx->data);
    } else {
        templateSet(template, "graph",
                    "<ul class=\"posts\"><div class=\"not-found\">Nothing "
                    "here.</div></ul>");
    }

    templateSet(template, "active", "dashboard");
    templateSet(template, "loggedIn", "t");
    templateSet(template, "subtitle", "Dashboard");
    templateSet(template, "accountName", my_acc->name->data);

    return template;
}
