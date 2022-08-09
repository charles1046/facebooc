#include "notFound.h"
#include "helper.h"
#include "template.h"

Response *notFound(const Request *req)
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
