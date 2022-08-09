#include "handler/post.h"
#include "db.h"
#include "helper.h"

#include "models/post.h"

// I posted a post via HTTP POST
Response *post(const Request *req)
{
    const Account *acc = get_account(req->cookies);
    if (unlikely(acc == NULL))
        goto fail;

    if (unlikely(req->method != POST))
        goto fail;

    Basic_string *postStr = Basic_string_init(body_get(req->postBody, "post"));

    if (postStr->size == 0) {
        Basic_string_delete(postStr);
        goto fail;
    } else if (postStr->size < MAX_BODY_LEN)
        postCreate_move(get_db(), acc->id, postStr);

fail:
    accountDel((Account *) acc);
    return responseNewRedirect("/dashboard/");
}