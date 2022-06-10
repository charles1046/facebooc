#include "handler/handlers/connect.h"

#include <stdio.h>

#include "db.h"
#include "handler/handler_utility.h"
#include "models/account.h"
#include "models/connection.h"
#include "utility.h"

Response* connect(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	char redirect_path[32] = { "/dashboard/" };
	if(unlikely(my_acc == NULL))
		goto fail;

	const Account* account = accountGetById(get_db(), get_id(req->uri));
	if(!account)
		goto fail;

	connectionDel(connectionCreate(get_db(), my_acc->id, account->id));

	snprintf(redirect_path, 32, "/profile/%d/", account->id);

fail:
	accountDel((Account*)my_acc);
	return responseNewRedirect(redirect_path);
}