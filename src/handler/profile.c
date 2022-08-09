#include "profile.h"

#include "db.h"
#include "helper.h"
#include "template.h"

#include "models/connection.h"
#include "models/like.h"
#include "models/post.h"

#include <stdio.h>
#include <time.h>

// Preview someone's profile page
Response *profile(const Request *req)
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
