#include "handler/handlers/post.h"

#include <string.h>

#include "db.h"
#include "models/account.h"
#include "models/post.h"
#include "utility.h"

Response* post(Request* req) {
	const Account* acc = get_account(req->cookies);
	if(unlikely(acc == NULL))
		goto fail;

	if(unlikely(req->method != POST))
		goto fail;

	const char* postStr = body_get(req->postBody, "post");

	if(strlen(postStr) == 0)
		goto fail;

	else if(strlen(postStr) < MAX_BODY_LEN)
		postDel(postCreate(get_db(), acc->id, postStr));

fail:
	accountDel((Account*)acc);
	return responseNewRedirect("/dashboard/");
}