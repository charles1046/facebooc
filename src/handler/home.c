#include "home.h"
#include "helper.h"
#include "template.h"

Response *home(const Request *req)
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