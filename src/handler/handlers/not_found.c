#include "handler/handlers/not_found.h"

#include "models/account.h"
#include "template.h"

Response* not_found(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	const int is_loggedIn = !!my_acc;
	accountDel((Account*)my_acc);

	Response* response = responseNew();
	Template* template = templateNew("templates/404.html");
	templateSet(template, "loggedIn", is_loggedIn ? "t" : "");
	templateSet(template, "subtitle", "404 Not Found");
	responseSetStatus(response, NOT_FOUND);
	responseSetBody(response, templateRender(template));
	templateDel(template);
	return response;
}