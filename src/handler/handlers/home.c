#include "handler/handlers/home.h"

#include "models/account.h"
#include "template.h"

Response* home(Request* req) {
	// ! What do here do?
	const Account* my_acc = get_account(req->cookies);
	if(my_acc) {
		accountDel((Account*)my_acc);
		return responseNewRedirect("/dashboard/");
	}

	Response* response = responseNew();
	Template* template = templateNew("templates/index.html");
	responseSetStatus(response, OK);
	templateSet(template, "active", "home");
	templateSet(template, "subtitle", "Home");
	responseSetBody(response, templateRender(template));
	templateDel(template);
	return response;
}