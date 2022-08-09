#include "handler/search.h"
#include "helper.h"

#include "db.h"
#include "template.h"

#include <stdio.h>

Response *search(const Request *req)
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