#include "handler/handlers/search.h"

#include <stdio.h>
#include <string.h>

#include "bs.h"
#include "db.h"
#include "handler/handler_utility.h"
#include "models/account.h"
#include "utility.h"

Response* search(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	if(unlikely(my_acc == NULL))
		return responseNewRedirect("/login/");
	accountDel((Account*)my_acc);

	const char* query = query_get(req->queryString, "q");

	if(!query)
		return NULL;

	char* res = NULL;
	char sbuff[1024];

	Node* accountCell = accountSearch(get_db(), query, 0);
	if(accountCell)
		res = bsNew("<ul class=\"search-results\">");

	Account* account = NULL;
	Node* accountPCell = NULL;

	while(accountCell) {
		account = (Account*)accountCell->value;

		const size_t name_len = strlen(account->name);
		const size_t email_len = strlen(account->email);

		snprintf(sbuff, 62 + name_len + email_len,
				 "<li><a href=\"/profile/%d/\">%s</a> (<span>%s</span>)</li>\n", account->id,
				 account->name, account->email);
		bsLCat(&res, sbuff);

		accountDel(account);
		accountPCell = accountCell;
		accountCell = (Node*)accountCell->next;

		free(accountPCell);
	}

	if(res)
		bsLCat(&res, "</ul>");

	Response* response = responseNew();
	Template* template = templateNew("templates/search.html");
	responseSetStatus(response, OK);

	if(!res) {
		templateSet(template, "results",
					"<h4 class=\"not-found\">There were no results "
					"for your query.</h4>");
	}
	else {
		templateSet(template, "results", res);
		bsDel(res);
	}

	templateSet(template, "searchQuery", query);
	templateSet(template, "active", "search");
	templateSet(template, "loggedIn", "t");
	templateSet(template, "subtitle", "Search");
	responseSetBody(response, templateRender(template));
	templateDel(template);
	return response;
}