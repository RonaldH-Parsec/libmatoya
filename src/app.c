// Copyright (c) Christopher D. Dickson <cdd@matoya.group>
//
// This Source Code Form is subject to the terms of the MIT License.
// If a copy of the MIT License was not distributed with this file,
// You can obtain one at https://spdx.org/licenses/MIT.html.

#include "app.h"

#include <stdio.h>
#include <math.h>


// GFX

GFX_CTX_PROTOTYPES(_gl_)
GFX_CTX_PROTOTYPES(_vk_)
GFX_CTX_PROTOTYPES(_d3d9_)
GFX_CTX_PROTOTYPES(_d3d11_)
GFX_CTX_PROTOTYPES(_d3d12_)
GFX_CTX_PROTOTYPES(_metal_)
GFX_CTX_DECLARE_TABLE()

MTY_Device *MTY_WindowGetDevice(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return NULL;

	return cmn->api != MTY_GFX_NONE ? GFX_CTX_API[cmn->api].get_device(cmn->gfx_ctx) : NULL;
}

MTY_Context *MTY_WindowGetContext(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return NULL;

	return cmn->api != MTY_GFX_NONE ? GFX_CTX_API[cmn->api].get_context(cmn->gfx_ctx) : NULL;
}

MTY_Surface *MTY_WindowGetSurface(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return NULL;

	return cmn->api != MTY_GFX_NONE ? GFX_CTX_API[cmn->api].get_surface(cmn->gfx_ctx) : NULL;
}

void MTY_WindowDrawQuad(MTY_App *app, MTY_Window window, const void *image, const MTY_RenderDesc *desc)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return;

	if (cmn->api != MTY_GFX_NONE)
		GFX_CTX_API[cmn->api].draw_quad(cmn->gfx_ctx, image, desc);
}

void MTY_WindowDrawUI(MTY_App *app, MTY_Window window, const MTY_DrawData *dd)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return;

	if (cmn->api != MTY_GFX_NONE)
		GFX_CTX_API[cmn->api].draw_ui(cmn->gfx_ctx, dd);
}

bool MTY_WindowHasUITexture(MTY_App *app, MTY_Window window, uint32_t id)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return false;

	return cmn->api != MTY_GFX_NONE && GFX_CTX_API[cmn->api].has_ui_texture(cmn->gfx_ctx, id);
}

bool MTY_WindowSetUITexture(MTY_App *app, MTY_Window window, uint32_t id, const void *rgba, uint32_t width, uint32_t height)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return false;

	return cmn->api != MTY_GFX_NONE && GFX_CTX_API[cmn->api].set_ui_texture(cmn->gfx_ctx, id, rgba, width, height);
}

void MTY_WindowPresent(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return;

	if (cmn->api != MTY_GFX_NONE)
		GFX_CTX_API[cmn->api].present(cmn->gfx_ctx);
}

MTY_GFX MTY_WindowGetGFX(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return MTY_GFX_NONE;

	return cmn->api;
}

bool MTY_WindowSetGFX(MTY_App *app, MTY_Window window, MTY_GFX api, bool vsync)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return false;

	if (cmn->api != MTY_GFX_NONE) {
		GFX_CTX_API[cmn->api].destroy(&cmn->gfx_ctx);
		cmn->api = MTY_GFX_NONE;
	}

	if (api != MTY_GFX_NONE) {
		void *native = MTY_WindowGetNative(app, window);

		if (!GFX_API_SUPPORTED(api))
			api = GFX_API_DEFAULT;

		cmn->gfx_ctx = GFX_CTX_API[api].create(native, vsync);

		// Fallback
		if (!cmn->gfx_ctx) {
			if (api == MTY_GFX_D3D12)
				return MTY_WindowSetGFX(app, window, MTY_GFX_D3D11, vsync);

			if (api == MTY_GFX_D3D11)
				return MTY_WindowSetGFX(app, window, MTY_GFX_D3D9, vsync);

			if (api == MTY_GFX_D3D9 || api == MTY_GFX_METAL)
				return MTY_WindowSetGFX(app, window, MTY_GFX_GL, vsync);

		} else {
			cmn->api = api;
		}
	}

	return cmn->gfx_ctx != NULL;
}


// WebView

static void webview_ready_func(MTY_App *app, MTY_Window window)
{
	MTY_Event evt = {
		.type = MTY_EVENT_WEBVIEW_READY,
		.window = window,
	};

	void *opaque = NULL;
	MTY_EventFunc event_func = mty_app_get_event_func(app, &opaque);

	event_func(&evt, opaque);
}

static void webview_text_func(MTY_App *app, MTY_Window window, const char *text)
{
	MTY_Event evt = {
		.type = MTY_EVENT_WEBVIEW_TEXT,
		.window = window,
		.webviewText = text,
	};

	void *opaque = NULL;
	MTY_EventFunc event_func = mty_app_get_event_func(app, &opaque);

	event_func(&evt, opaque);
}

static void webview_key_func(MTY_App *app, MTY_Window window, bool pressed, MTY_Key key, MTY_Mod mods)
{
	MTY_Event evt = {
		.type = MTY_EVENT_WEBVIEW_KEY,
		.window = window,
		.key.pressed = pressed,
		.key.key = key,
		.key.mod = mods,
	};

	mty_app_kb_to_hotkey(app, &evt, MTY_EVENT_WEBVIEW_HOTKEY);

	void *opaque = NULL;
	MTY_EventFunc event_func = mty_app_get_event_func(app, &opaque);

	event_func(&evt, opaque);
}

bool MTY_WindowSetWebView(MTY_App *app, MTY_Window window, const char *dir, const char *source,
	MTY_WebViewFlag flags)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn || cmn->webview)
		return false;

	cmn->webview = mty_webview_create(app, window, dir, source, flags,
		webview_ready_func, webview_text_func, webview_key_func);

	return cmn->webview != NULL;
}

void MTY_WindowRemoveWebView(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn)
		return;

	mty_webview_destroy(&cmn->webview);
}

bool MTY_WebViewExists(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);

	return cmn && cmn->webview;
}

void MTY_WebViewShow(MTY_App *app, MTY_Window window, bool show)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn || !cmn->webview)
		return;

	mty_webview_show(cmn->webview, show);
}

bool MTY_WebViewIsVisible(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn || !cmn->webview)
		return false;

	return mty_webview_is_visible(cmn->webview);
}

void MTY_WebViewSendText(MTY_App *app, MTY_Window window, const char *text)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn || !cmn->webview)
		return;

	mty_webview_send_text(cmn->webview, text);
}

void MTY_WebViewReload(MTY_App *app, MTY_Window window)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn || !cmn->webview)
		return;

	mty_webview_reload(cmn->webview);
}

void MTY_WebViewSetInputPassthrough(MTY_App *app, MTY_Window window, bool passthrough)
{
	struct window_common *cmn = mty_window_get_common(app, window);
	if (!cmn || !cmn->webview)
		return;

	mty_webview_set_input_passthrough(cmn->webview, passthrough);
}


// Window sizing

MTY_Frame mty_window_adjust(uint32_t screen_w, uint32_t screen_h, float scale, float max_h,
	int32_t x, int32_t y, uint32_t w, uint32_t h)
{
	if (h * scale > max_h * screen_h) {
		float aspect = (float) w / h;
		h = lrint(max_h * screen_h / scale);
		w = lrint(h * aspect);
	}

	if (screen_w > w) {
		x += (screen_w - w) / 2;

	} else {
		x = 0;
		w = screen_w;
	}

	if (screen_h > h) {
		y += (screen_h - h) / 2;

	} else {
		y = 0;
		h = screen_h;
	}

	return (MTY_Frame) {
		.x = x,
		.y = y,
		.size.w = w,
		.size.h = h,
	};
}


// Hotkeys

void mty_app_kb_to_hotkey(MTY_App *app, MTY_Event *evt, MTY_EventType type)
{
	MTY_Hash *h = mty_app_get_hotkey_hash(app);

	MTY_Mod mod = evt->key.mod & 0xFF;
	uint32_t hotkey = (uint32_t) (uintptr_t) MTY_HashGetInt(h, (mod << 16) | evt->key.key);

	if (hotkey != 0) {
		if (evt->key.pressed) {
			evt->type = type;
			evt->hotkey = hotkey;

		} else {
			evt->type = MTY_EVENT_NONE;
		}
	}
}


// Event utility

void MTY_PrintEvent(const MTY_Event *evt)
{
	#define PFMT(name, evt, fmt, ...) \
		printf("[%d] %-21s" fmt "\n", evt->window, #name, ## __VA_ARGS__)

	#define PEVENT(name, evt, fmt, ...) \
		case name: PFMT(name, evt, fmt, ## __VA_ARGS__); break

	switch (evt->type) {
		PEVENT(MTY_EVENT_CLOSE, evt, "");
		PEVENT(MTY_EVENT_QUIT, evt, "");
		PEVENT(MTY_EVENT_SHUTDOWN, evt, "");
		PEVENT(MTY_EVENT_CLIPBOARD, evt, "");
		PEVENT(MTY_EVENT_WEBVIEW_READY, evt, "");
		PEVENT(MTY_EVENT_SIZE, evt, "");
		PEVENT(MTY_EVENT_MOVE, evt, "");
		PEVENT(MTY_EVENT_BACK, evt, "");

		PEVENT(MTY_EVENT_KEY, evt,
			"key: 0x%X, mod: 0x%X, pressed: %u",
			evt->key.key, evt->key.mod, evt->key.pressed);

		PEVENT(MTY_EVENT_WEBVIEW_KEY, evt,
			"key: 0x%X, mod: 0x%X, pressed: %u",
			evt->key.key, evt->key.mod, evt->key.pressed);

		PEVENT(MTY_EVENT_HOTKEY, evt,
			"id: %u",
			evt->hotkey);

		PEVENT(MTY_EVENT_WEBVIEW_HOTKEY, evt,
			"id: %u",
			evt->hotkey);

		PEVENT(MTY_EVENT_TEXT, evt,
			"text: %s",
			evt->text);

		PEVENT(MTY_EVENT_SCROLL, evt,
			"x: %d, y: %d, pixels: %u",
			evt->scroll.x, evt->scroll.y, evt->scroll.pixels);

		PEVENT(MTY_EVENT_FOCUS, evt,
			"focus: %u",
			evt->focus);

		PEVENT(MTY_EVENT_MOTION, evt,
			"x: %d, y: %d, relative: %u, synth: %u",
			evt->motion.x, evt->motion.y, evt->motion.relative, evt->motion.synth);

		PEVENT(MTY_EVENT_BUTTON, evt,
			"x: %d, y: %d, button: %u, pressed: %u",
			evt->button.x, evt->button.y, evt->button.button, evt->button.pressed);

		PEVENT(MTY_EVENT_DROP, evt,
			"name: %s, buf: %p, size: %zu",
			evt->drop.name, evt->drop.buf, evt->drop.size);

		PEVENT(MTY_EVENT_TRAY, evt,
			"id: %u",
			evt->trayID);

		PEVENT(MTY_EVENT_REOPEN, evt,
			"arg: %s",
			evt->reopenArg);

		PEVENT(MTY_EVENT_PEN, evt,
			"x: %u, y: %u, flags: 0x%X, pressure: %u, rotation: %u, tiltX: %d, tiltY: %d",
			evt->pen.x, evt->pen.y, evt->pen.flags, evt->pen.pressure, evt->pen.rotation,
			evt->pen.tiltX, evt->pen.tiltY);

		PEVENT(MTY_EVENT_WEBVIEW_TEXT, evt,
			"webviewText: %s",
			evt->webviewText);

		PEVENT(MTY_EVENT_CONNECT, evt,
			"id: %u",
			evt->controller.id);

		PEVENT(MTY_EVENT_DISCONNECT, evt,
			"id: %u",
			evt->controller.id);

		case MTY_EVENT_CONTROLLER: {
			PFMT(MTY_EVENT_CONTROLLER, evt,
				"id: %u, type: %u, vid-pid: %04X-%04X, numButtons: %u, numAxes: %u",
				evt->controller.id, evt->controller.type, evt->controller.vid, evt->controller.pid,
				evt->controller.numButtons, evt->controller.numAxes);

			printf("  buttons: ");
			for (uint8_t x = 0; x < evt->controller.numButtons; x++)
				printf("%u", evt->controller.buttons[x]);
			printf("\n");

			printf("  axes: ");
			for (uint8_t x = 0; x < evt->controller.numAxes; x++) {
				const MTY_Axis *a = &evt->controller.axes[x];
				printf("[%X] %-7d", a->usage, a->value);
			}
			printf("\n");
		}
	}
}
