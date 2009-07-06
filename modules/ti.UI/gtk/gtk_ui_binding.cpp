/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"
#include "../url/url.h"
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#include <gdk/gdkx.h>
#include <Poco/Thread.h>

namespace ti
{
	GtkUIBinding::GtkUIBinding(Host *host) :
		UIBinding(host),
		evaluator(new ScriptEvaluator()),
		menu(0),
		contextMenu(0),
		iconPath("")
	{
		/* Prepare the custom URL handlers */
		webkit_titanium_set_normalize_url_cb(NormalizeURLCallback);
		webkit_titanium_set_url_to_file_url_cb(URLToFileURLCallback);

		/* Register the script evaluator */
		webkit_titanium_add_script_evaluator(evaluator);

		char buf[256];
		snprintf(buf, 256, "%s/%s", PRODUCT_NAME, STRING(PRODUCT_VERSION));
		g_set_prgname(buf);

		std::string webInspectorPath = host->GetRuntimePath();
		webInspectorPath = FileUtils::Join(webInspectorPath.c_str(), "webinspector", NULL);
		webkit_titanium_set_inspector_url(webInspectorPath.c_str());

		// Tell Titanium what WebKit is using for a user-agent
		SharedKObject global = host->GetGlobalObject();
		const gchar* user_agent = webkit_titanium_get_user_agent();
		global->Set("userAgent", Value::NewString(user_agent));
	}

	SharedUserWindow GtkUIBinding::CreateWindow(
		WindowConfig* config,
		SharedUserWindow& parent)
	{
		UserWindow* w = new GtkUserWindow(config, parent);
		return w->GetSharedPtr();
	}

	void GtkUIBinding::ErrorDialog(std::string msg)
	{
		GtkWidget* dialog = gtk_message_dialog_new(
			NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			"%s",
			msg.c_str());
		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);
		UIBinding::ErrorDialog(msg);
	}

	SharedMenu GtkUIBinding::CreateMenu()
	{
		return new GtkMenu();
	}

	SharedMenuItem GtkUIBinding::CreateMenuItem(
		std::string label, SharedKMethod callback, std::string iconURL)
	{
		return new GtkMenuItem(MenuItem::NORMAL, label, callback, iconURL);
	}

	SharedMenuItem GtkUIBinding::CreateSeparatorMenuItem()
	{
		return new GtkMenuItem(MenuItem::SEPARATOR, std::string(), NULL, std::string());
	}

	SharedMenuItem GtkUIBinding::CreateCheckMenuItem(
		std::string label, SharedKMethod callback)
	{
		return new GtkMenuItem(MenuItem::CHECK, label, callback, std::string());
	}

	void GtkUIBinding::SetMenu(SharedMenu newMenu) {
		this->menu = newMenu.cast<GtkMenu>();

		// Notify all windows that the app menu has changed.
		std::vector<SharedUserWindow>& windows = this->GetOpenWindows();
		std::vector<SharedUserWindow>::iterator i = windows.begin();
		while (i != windows.end()) {
			SharedPtr<GtkUserWindow> guw = (*i).cast<GtkUserWindow>();
			if (!guw.isNull())
				guw->AppMenuChanged();
			i++;
		}
	}

	void GtkUIBinding::SetContextMenu(SharedMenu newMenu)
	{
		this->contextMenu = newMenu.cast<GtkMenu>();
	}

	void GtkUIBinding::SetIcon(std::string& iconPath)
	{
		this->iconPath = iconPath;

		// Notify all windows that the app icon has changed.
		std::vector<SharedUserWindow>& windows = this->GetOpenWindows();
		std::vector<SharedUserWindow>::iterator i = windows.begin();
		while (i != windows.end())
		{
			SharedPtr<GtkUserWindow> guw = (*i).cast<GtkUserWindow>();
			if (!guw.isNull())
				guw->AppIconChanged();
			i++;
		}
	}

	SharedTrayItem GtkUIBinding::AddTray(SharedString iconPath, SharedKMethod cb)
	{
		SharedTrayItem item = new GtkTrayItem(iconPath, cb);
		return item;
	}

	long GtkUIBinding::GetIdleTime()
	{
		Display *display = gdk_x11_get_default_xdisplay();
		if (display == NULL)
			return -1;
		int screen = gdk_x11_get_default_screen();

		XScreenSaverInfo *mit_info = XScreenSaverAllocInfo();
		XScreenSaverQueryInfo(display, RootWindow(display, screen), mit_info);
		long idle_time = mit_info->idle;
		XFree(mit_info);

		return idle_time;
	}

	SharedMenu GtkUIBinding::GetMenu()
	{
		return this->menu;
	}

	SharedMenu GtkUIBinding::GetContextMenu()
	{
		return this->contextMenu;
	}

	std::string& GtkUIBinding::GetIcon()
	{
		return this->iconPath;
	}

}
