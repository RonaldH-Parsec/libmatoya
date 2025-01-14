// Copyright (c) Christopher D. Dickson <cdd@matoya.group>
//
// This Source Code Form is subject to the terms of the MIT License.
// If a copy of the MIT License was not distributed with this file,
// You can obtain one at https://spdx.org/licenses/MIT.html.

#include "webview.h"

struct webview *mty_webview_create(MTY_App *app, MTY_Window window, const char *dir,
	const char *source, MTY_WebViewFlag flags, WEBVIEW_READY ready_func, WEBVIEW_TEXT text_func,
	WEBVIEW_KEY key_func)
{
	return NULL;
}

void mty_webview_destroy(struct webview **webview)
{
}

void mty_webview_show(struct webview *ctx, bool show)
{
}

bool mty_webview_is_visible(struct webview *ctx)
{
	return false;
}

void mty_webview_send_text(struct webview *ctx, const char *msg)
{
}

void mty_webview_reload(struct webview *ctx)
{
}

void mty_webview_set_input_passthrough(struct webview *ctx, bool passthrough)
{
}

void mty_webview_update_size(struct webview *ctx)
{
}

bool mty_webview_was_hidden_during_keydown(struct webview *ctx)
{
	return false;
}
