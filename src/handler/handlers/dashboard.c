#include "handler/handlers/dashboard.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "bs.h"
#include "db.h"
#include "list.h"
#include "models/account.h"
#include "models/like.h"
#include "models/post.h"
#include "template.h"
#include "utility.h"

Response* dashboard(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	if(unlikely(my_acc == NULL))
		return NULL;

	Node* postCell = postGetLatestGraph(get_db(), my_acc->id, 0);
	char* res = NULL;
	if(postCell)
		res = bsNew("<ul class=\"posts\">");

	// ! What do here do?
	while(postCell) {
		Post* post = (Post*)postCell->value;
		Account* account = accountGetById(get_db(), post->authorId);
		bool liked = likeLiked(get_db(), my_acc->id, post->id);
		const size_t acc_len = strlen(account->name);
		const size_t post_len = bsGetLen(post->body);

		char* bbuff = bsNewLen("", strlen(post->body) + 256);
		snprintf(bbuff, 86 + acc_len + post_len,
				 "<li class=\"post-item\">"
				 "<div class=\"post-author\">%s</div>"
				 "<div class=\"post-content\">"
				 "%s"
				 "</div>",
				 account->name, post->body);
		accountDel(account);
		bsLCat(&res, bbuff);

		char sbuff[128];
		if(liked) {
			snprintf(sbuff, 55, "<a class=\"btn\" href=\"/unlike/%d/\">Liked</a> - ", post->id);
			bsLCat(&res, sbuff);
		}
		else {
			snprintf(sbuff, 52, "<a class=\"btn\" href=\"/like/%d/\">Like</a> - ", post->id);
			bsLCat(&res, sbuff);
		}

		time_t t = post->createdAt;
		struct tm* info = gmtime(&t);
		info->tm_hour = info->tm_hour + 8;
		strftime(sbuff, 128, "%c GMT+8", info);
		bsLCat(&res, sbuff);
		bsLCat(&res, "</li>");

		bsDel(bbuff);
		postDel(post);
		Node* postPCell = postCell;
		postCell = postCell->next;

		free(postPCell);
	}

	Template* template = templateNew("templates/dashboard.html");
	if(res) {
		bsLCat(&res, "</ul>");
		templateSet(template, "graph", res);
		bsDel(res);
	}
	else {
		templateSet(template, "graph",
					"<ul class=\"posts\"><div class=\"not-found\">Nothing "
					"here.</div></ul>");
	}

	templateSet(template, "active", "dashboard");
	templateSet(template, "loggedIn", "t");
	templateSet(template, "subtitle", "Dashboard");
	templateSet(template, "accountName", my_acc->name);
	Response* response = responseNew();
	responseSetStatus(response, OK);
	responseSetBody(response, templateRender(template));
	templateDel(template);
	accountDel((Account*)my_acc);
	return response;
}