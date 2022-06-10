#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bs.h"
#include "db.h"
#include "handler/handler_utility.h"
#include "handler/handlers/profile.h"
#include "models/account.h"
#include "models/connection.h"
#include "models/like.h"
#include "models/post.h"
#include "template.h"
#include "utility.h"

Response* profile(Request* req) {
	// Check if I'm logged in
	const Account* my_acc = get_account(req->cookies);
	if(unlikely(my_acc == NULL))
		return NULL;

	const Account* acc2 = accountGetById(get_db(), get_id(req->uri));

	if(!acc2)
		return NULL;
	if(acc2->id == my_acc->id) {
		accountDel((Account*)my_acc);
		accountDel((Account*)acc2);
		return responseNewRedirect("/dashboard/");
	}

	// VLA is not in POSIX1.
	// Max length of uid is len(INT_MAX), in other words, the maxlen of uid is 10
	char acc2_id_str[11] = { 0 };
	snprintf(acc2_id_str, 11, "%d", acc2->id);

	Connection* connection = connectionGetByAccountIds(get_db(), my_acc->id, acc2->id);
	char connectStr[512] = { 0 };
	if(connection) {
		snprintf(connectStr, 26 + strlen(acc2->name), "You and %s are connected!", acc2->name);
	}
	else {
		const size_t name_len = strlen(acc2->name);
		snprintf(connectStr, 76 + name_len + 10,
				 "You and %s are not connected."
				 " <a href=\"/connect/%d/\">Click here</a> to connect!",
				 acc2->name, acc2->id);
	}
	connectionDel(connection);

	Node* postPCell = NULL;
	Node* postCell = postGetLatest(get_db(), acc2->id, 0);

	char* res = NULL;
	if(postCell)
		res = bsNew("<ul class=\"posts\">");
	bool liked;
	char sbuff[128];
	char* bbuff = NULL;
	time_t t;
	while(postCell) {
		Post* post = (Post*)postCell->value;
		liked = likeLiked(get_db(), my_acc->id, post->id);

		const size_t body_len = strlen(post->body);
		bbuff = bsNewLen("", body_len + 256);
		snprintf(bbuff, 54 + body_len,
				 "<li class=\"post-item\"><div class=\"post-author\">%s</div>", post->body);
		bsLCat(&res, bbuff);

		if(liked) {
			bsLCat(&res, "Liked - ");
		}
		else {
			snprintf(sbuff, 52, "<a class=\"btn\" href=\"/like/%d/\">Like</a> - ", post->id);
			bsLCat(&res, sbuff);
		}

		t = post->createdAt;
		strftime(sbuff, 128, "%c GMT", gmtime(&t));
		bsLCat(&res, sbuff);
		bsLCat(&res, "</li>");

		bsDel(bbuff);
		postDel(post);
		postPCell = postCell;
		postCell = (Node*)postCell->next;

		free(postPCell);
	}
	Template* template = templateNew("templates/profile.html");
	if(res) {
		bsLCat(&res, "</ul>");
		templateSet(template, "profilePosts", res);
		bsDel(res);
	}
	else {
		templateSet(template, "profilePosts",
					"<h4 class=\"not-found\">This person has not posted "
					"anything yet!</h4>");
	}

	templateSet(template, "active", "profile");
	templateSet(template, "loggedIn", "t");
	templateSet(template, "subtitle", acc2->name);
	templateSet(template, "profileId", acc2_id_str);
	templateSet(template, "profileName", acc2->name);
	templateSet(template, "profileEmail", acc2->email);
	templateSet(template, "profileConnect", connectStr);
	templateSet(template, "accountName", my_acc->name);
	Response* response = responseNew();
	responseSetStatus(response, OK);
	responseSetBody(response, templateRender(template));
	accountDel((Account*)my_acc);
	accountDel((Account*)acc2);
	templateDel(template);
	return response;
}