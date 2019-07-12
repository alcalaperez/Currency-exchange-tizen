#include <tizen.h>
#include "chfexchange.h"
#include <curl/curl.h>
#include <net_connection.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-glib/json-glib.h>
#include <glib-object.h>
#include <strings.h>
#include <dlog.h>

typedef struct widget_instance_data {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *valueOfExchange;
	Evas_Object *currencies;
	Evas_Object *button_update;
} widget_instance_data_s;

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size,
		size_t nmemb, std::string *s) {
	size_t newLength = size * nmemb;
	s->append((char*) contents, newLength);
	return newLength;
}

widget_instance_data_s *wid = (widget_instance_data_s*) malloc(
		sizeof(widget_instance_data_s));
Evas_Object *list;
Evas_Object *box;
Evas_Object *list1;
Evas_Object *box1;

Elm_Object_Item *fromcurr;
Elm_Object_Item *tocurr;

void update_cb(void *data, Evas_Object *obj, void *event_info) {
	elm_object_text_set(wid->valueOfExchange, "......");

	//Forming the URL with the 2 selected currencies
	std::string url;
	url.append("https://free.currconv.com/api/v7/convert?q=");
	url.append(elm_object_item_text_get(fromcurr));
	url.append("_");
	url.append(elm_object_item_text_get(tocurr));
	url.append("&compact=ultra&apiKey=yourApiKey");

	//JSON object to look in the response
	std::string exchange_code;
	exchange_code.append(elm_object_item_text_get(fromcurr));
	exchange_code.append("_");
	exchange_code.append(elm_object_item_text_get(tocurr));

	//Forming the tittle
	std::string label_code;
	label_code.append(elm_object_item_text_get(fromcurr));
	label_code.append(" to ");
	label_code.append(elm_object_item_text_get(tocurr));

	CURL *curl;
	CURLcode res;
	std::string response;
	curl = curl_easy_init();

	connection_h connection;
	int conn_err;
	conn_err = connection_create(&connection);

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
				CurlWrite_CallbackFunc_StdString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		char *proxy_address;
		conn_err = connection_get_proxy(connection,
				CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);

		if (conn_err == CONNECTION_ERROR_NONE && proxy_address) {
			curl_easy_setopt(curl, CURLOPT_PROXY, proxy_address);
		}

		res = curl_easy_perform(curl);


		JsonParser* jsonParser = NULL;
		jsonParser = json_parser_new();
		gboolean result = json_parser_load_from_data(jsonParser,
				response.c_str(), response.size(), NULL);
		JsonNode *root;

		if (result) {
			root = json_parser_get_root(jsonParser);

			// A new JsonObject variable for loading root object of the json.
			JsonObject *object;

			// Get All the objects in the Node. In this case 2 Object ["data" + "total"].
			object = json_node_get_object(root);
			auto str = std::to_string(
					json_object_get_double_member(object,
							exchange_code.c_str()));
			elm_object_text_set(wid->valueOfExchange, str.c_str());
			elm_object_text_set(wid->currencies, label_code.c_str());
		}

		curl_easy_cleanup(curl);
		evas_object_show(wid->valueOfExchange);
		evas_object_show(wid->currencies);

	}
}

/* Called when the list item is selected */
static void _selected_item_cb_2(void *data, Evas_Object *obj,
		void *event_info) {
	tocurr = elm_list_selected_item_get(obj);
	evas_object_hide(box1);
	evas_object_show(wid->button_update);

}

/* Called when the list item is selected */
static void _selected_item_cb_1(void *data, Evas_Object *obj,
		void *event_info) {
	fromcurr = elm_list_selected_item_get(obj);

	evas_object_hide(box);

	//create a box for your list object
	box1 = elm_box_add(wid->win);
	evas_object_size_hint_weight_set(box1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(wid->win, box1);
	evas_object_show(box1);

	//create the list object
	list1 = elm_list_add(wid->win);
	evas_object_size_hint_weight_set(list1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list1, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(box1, list1);

	elm_list_item_append(list1, "EUR", NULL, NULL, _selected_item_cb_2, NULL);
	elm_list_item_append(list1, "CHF", NULL, NULL, _selected_item_cb_2, NULL);
	elm_list_item_append(list1, "USD", NULL, NULL, _selected_item_cb_2, NULL);

	// enable scroller bouncing
	elm_scroller_bounce_set(list1, EINA_TRUE, EINA_TRUE);

	evas_object_show(list1);
	elm_list_go(list1);

}

static int widget_instance_create(widget_context_h context, bundle *content,
		int w, int h, void *user_data) {
	int ret;


	/* Window */
	ret = widget_app_get_elm_win(context, &wid->win);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
		return WIDGET_ERROR_FAULT;
	}

	evas_object_resize(wid->win, w, h);

	/* Conformant */
	wid->conform = elm_conformant_add(wid->win);
	evas_object_size_hint_weight_set(wid->conform, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	elm_win_resize_object_add(wid->win, wid->conform);
	evas_object_show(wid->conform);

	/* Label*/
	wid->valueOfExchange = elm_label_add(wid->conform);
	evas_object_resize(wid->valueOfExchange, w, h / 3);
	evas_object_move(wid->valueOfExchange, w / 3.2, h / 2);

	wid->currencies = elm_label_add(wid->conform);
	evas_object_resize(wid->currencies, w, h / 3);
	evas_object_move(wid->currencies, w / 3.8, h / 3);
	elm_object_text_set(wid->currencies, "CHF to EUR");

	//create a box for your list object
	box = elm_box_add(wid->win);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(wid->win, box);
	evas_object_show(box);

	//create the list object
	list = elm_list_add(wid->win);
	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(box, list);

	elm_list_item_append(list, "EUR", NULL, NULL, _selected_item_cb_1, NULL);
	elm_list_item_append(list, "CHF", NULL, NULL, _selected_item_cb_1, NULL);
	elm_list_item_append(list, "USD", NULL, NULL, _selected_item_cb_1, NULL);

	// enable scroller bouncing
	elm_scroller_bounce_set(list, EINA_TRUE, EINA_TRUE);
	evas_object_show(list);
	elm_list_go(list);

	wid->button_update = elm_button_add(wid->win);
	elm_object_style_set(wid->button_update, "bottom");
	evas_object_resize(wid->button_update, w, h / 6);
	elm_object_text_set(wid->button_update, "Update");
	evas_object_smart_callback_add(wid->button_update, "clicked", update_cb, wid);

	/* Show window after base gui is set up */
	evas_object_show(wid->win);

	widget_app_context_set_tag(context, wid);
	return WIDGET_ERROR_NONE;
}

static int widget_instance_destroy(widget_context_h context,
		widget_app_destroy_type_e reason, bundle *content, void *user_data) {
	widget_instance_data_s *wid = NULL;
	widget_app_context_get_tag(context, (void**) &wid);

	if (wid->win)
		evas_object_del(wid->win);

	free(wid);

	return WIDGET_ERROR_NONE;
}

static int widget_instance_pause(widget_context_h context, void *user_data) {
	/* Take necessary actions when widget instance becomes invisible. */
	return WIDGET_ERROR_NONE;

}

static int widget_instance_resume(widget_context_h context, void *user_data) {
	/* Take necessary actions when widget instance becomes visible. */
	return WIDGET_ERROR_NONE;
}

static int widget_instance_update(widget_context_h context, bundle *content,
		int force, void *user_data) {
	/* Take necessary actions when widget instance should be updated. */
	return WIDGET_ERROR_NONE;
}

static int widget_instance_resize(widget_context_h context, int w, int h,
		void *user_data) {
	/* Take necessary actions when the size of widget instance was changed. */
	return WIDGET_ERROR_NONE;
}

static void widget_app_lang_changed(app_event_info_h event_info,
		void *user_data) {
	/* APP_EVENT_LANGUAGE_CHANGED */
	char *locale = NULL;
	app_event_get_language(event_info, &locale);
	elm_language_set(locale);
	free(locale);
}

static void widget_app_region_changed(app_event_info_h event_info,
		void *user_data) {
	/* APP_EVENT_REGION_FORMAT_CHANGED */
}

static widget_class_h widget_app_create(void *user_data) {
	/* Hook to take necessary actions before main event loop starts.
	 Initialize UI resources.
	 Make a class for widget instance.
	 */
	app_event_handler_h handlers[5] = { NULL, };

	widget_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
			APP_EVENT_LANGUAGE_CHANGED, widget_app_lang_changed, user_data);
	widget_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
			APP_EVENT_REGION_FORMAT_CHANGED, widget_app_region_changed,
			user_data);

	widget_instance_lifecycle_callback_s ops = { .create =
			widget_instance_create, .destroy = widget_instance_destroy, .pause =
			widget_instance_pause, .resume = widget_instance_resume, .update =
			widget_instance_update, .resize = widget_instance_resize, };

	return widget_app_class_create(ops, user_data);
}

static void widget_app_terminate(void *user_data) {
	/* Release all resources. */
}

int main(int argc, char *argv[]) {
	widget_app_lifecycle_callback_s ops = { 0, };
	int ret;

	ops.create = widget_app_create;
	ops.terminate = widget_app_terminate;

	ret = widget_app_main(argc, argv, &ops, NULL);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "widget_app_main() is failed. err = %d",
				ret);
	}

	return ret;
}

