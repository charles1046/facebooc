#include "handler/handlers/like.h"

#include <stdio.h>

#include "db.h"
#include "handler/handler_utility.h"
#include "models/account.h"
#include "models/like.h"
#include "models/post.h"
#include "utility.h"

Response* like(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	char redir_to[32] = { "/dashboard/" };
	if(unlikely(my_acc == NULL))
		goto fail;

	const Post* post = postGetById(get_db(), get_id(req->uri));
	if(!post)
		goto fail;

	likeDel(likeCreate(get_db(), my_acc->id, post->authorId, post->id));

	if(query_get(req->queryString, "r"))
		snprintf(redir_to, 32, "/profile/%d/", post->authorId);

fail:
	accountDel((Account*)my_acc);
	return responseNewRedirect(redir_to);
}