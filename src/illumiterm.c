/* Copyright 2023 Elijah Gordon (SLcK) <braindisassemblue@gmail.com>

*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.

*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.

*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <vte/vte.h>
#include <gtk/gtk.h>

const gchar* GetNewWindowTitle(VteTerminal* terminal) {
    return vte_terminal_get_window_title(terminal);
}

void SetWindowTitle(GtkWidget* window, const gchar* newTitle) {
    gtk_window_set_title(GTK_WINDOW(window), newTitle);
}

void WindowTitleChanged(GtkWidget* widget, gpointer window) {
    const gchar* new_title = GetNewWindowTitle(VTE_TERMINAL(widget));
    SetWindowTitle(GTK_WIDGET(window), new_title);
}

void SetExitStatus(GApplicationCommandLine* cli, gint status) {
    if (cli != NULL) {
        g_application_command_line_set_exit_status(cli, status);
        g_object_unref(cli);
    }
}

void DestroyWindow(GtkWidget* widget, gpointer data) {
    if (widget != NULL) {
        gtk_widget_destroy(widget);
    }
}

void DestroyAndQuit(GtkWidget* window, gint status) {
    GApplicationCommandLine* cli = g_object_get_data(G_OBJECT(window), "cli");
    if (cli != NULL) {
        SetExitStatus(cli, status);
    }
    DestroyWindow(window, NULL);
}

void HandleChildExit(GtkWidget* window, gint status) {
    DestroyAndQuit(window, status);
}

gboolean ChildExited(VteTerminal* term, gint status, gpointer data) {
    GtkWidget* window = GTK_WIDGET(data);
    HandleChildExit(window, status);
    return TRUE;
}

void NewWindow(void) {
    GError *error = NULL;
    gchar *argv[] = {"illumiterm", NULL};

    gboolean success = g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);

    if (!success) {
        g_error_free(error);
    }
}

void NewTab(void) {
    g_print("NewTab\n");
}

void Copy(void) {
    g_print("Copy\n");
}

void Paste(void) {
    g_print("Paste\n");
}

void ClearScrollback(void) {
    g_print("ClearScrollback\n");
}

void NameTab(void) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Name Tab", NULL, GTK_DIALOG_MODAL, "Cancel", GTK_RESPONSE_CANCEL, "OK", GTK_RESPONSE_OK, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
    gtk_widget_show(entry);

    gchar *username = getlogin();
    if (username != NULL) {
        gchar hostname[1024];
        gethostname(hostname, sizeof(hostname));
        gchar *user_host = g_strdup_printf("%s@%s", username, hostname);
        gtk_entry_set_text(GTK_ENTRY(entry), user_host);
        g_free(user_host);
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry), "Default Title");
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        const gchar *home_dir = g_get_home_dir();
        if (g_str_has_prefix(cwd, home_dir)) {
            gchar *relative_path = g_strdup_printf("~%s", cwd + strlen(home_dir));
            gchar *title_text = g_strdup_printf("%s:%s", gtk_entry_get_text(GTK_ENTRY(entry)), relative_path);
            gtk_entry_set_text(GTK_ENTRY(entry), title_text);
            g_free(relative_path);
            g_free(title_text);
        } else {
            gchar *title_text = g_strdup_printf("%s:%s", gtk_entry_get_text(GTK_ENTRY(entry)), cwd);
            gtk_entry_set_text(GTK_ENTRY(entry), title_text);
            g_free(title_text);
        }
    }

    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        const gchar *name = gtk_entry_get_text(GTK_ENTRY(entry));
        gtk_window_set_title(GTK_WINDOW(dialog), name);
    }

    gtk_widget_destroy(dialog);
}

void PreviousTab(void) {
    g_print("PreviousTab\n");
}

void NextTab(void) {
    g_print("NextTab\n");
}

void MoveTabLeft(void) {
    g_print("MoveTabLeft\n");
}

void MoveTabRight(void) {
    g_print("MoveTabRight\n");
}

void CloseTab(void) {
    g_print("CloseTab\n");
}

int NumTabs = 0;

void UpdateNumTabs(GtkWidget* TabContainer) {
    GList* children = gtk_container_get_children(GTK_CONTAINER(TabContainer));
    NumTabs = g_list_length(children);
    g_list_free(children);
}

gboolean ConfirmExit(GtkWidget* widget, GdkEvent* event, gpointer data) {
    GtkWidget* TabContainer = GTK_WIDGET(data);
    UpdateNumTabs(TabContainer);

    gchar* message = g_strdup_printf("You are about to close %d tabs. Are you sure you want to continue?", NumTabs);
    GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", message);

    gtk_window_set_title(GTK_WINDOW(dialog), "Confirm Close");
    gtk_window_set_deletable(GTK_WINDOW(dialog), FALSE);

    g_free(message);

    gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "/usr/share/icons/hicolor/48x48/apps/illumiterm.png", NULL);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return (response == GTK_RESPONSE_NO) ? TRUE : FALSE;
}

GtkWidget* ContextMenuHelper(GtkWidget* menu, const gchar* imagePath, const gchar* labelText, GCallback callback) {
    GtkWidget *item, *box, *icon, *label;
    
    item = gtk_menu_item_new();
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    icon = gtk_image_new_from_file(imagePath);
    label = gtk_label_new(labelText);
    
    GtkWidget* spacing = gtk_label_new("  ");
    
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), spacing, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(item), box);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    g_signal_connect(item, "activate", callback, NULL);

    return item;
}

GtkWidget* ContextMenu() {
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *separator;

    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/window-new.svg", "New Window", G_CALLBACK(NewWindow));
    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/tab-new.svg", "New Tab", G_CALLBACK(NewTab));

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);

    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/edit-copy.svg", "Copy", G_CALLBACK(Copy));
    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/edit-paste.svg", "Paste", G_CALLBACK(Paste));

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);

    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/edit-clear.svg", "Clear Scrollback", G_CALLBACK(ClearScrollback));
    
    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    
    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/edit.svg", "Name Tab", G_CALLBACK(NameTab));

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);

    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/go-previous.svg", "Previous Tab", G_CALLBACK(PreviousTab));
    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/go-next.svg", "Next Tab", G_CALLBACK(NextTab));
    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/go-up.svg", "Move Tab Left", G_CALLBACK(MoveTabLeft));
    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/go-down.svg", "Move Tab Right", G_CALLBACK(MoveTabRight));

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);

    ContextMenuHelper(menu, "/usr/share/icons/hicolor/24x24/apps/window-close.svg", "Close Tab", G_CALLBACK(CloseTab));

    gtk_widget_show_all(menu);

    return menu;
}

gboolean ButtonPressEvent(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->button != GDK_BUTTON_SECONDARY) {
        return FALSE;
    }
    GtkWidget *menu = ContextMenu();
    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);

    return TRUE;
}

void ChildReady(VteTerminal* terminal, GPid pid, GError* error, gpointer user_data) {
    if (terminal == NULL) {
        return;
    }
    if (pid == 0) {
        GtkWidget* window = GTK_WIDGET(user_data);
        gint error_code = (error != NULL) ? error->code : 0;
        DestroyAndQuit(window, error_code);
    }
}

gchar** GetEnviroment(GApplicationCommandLine* cli) {
    const gchar* const* environment = g_application_command_line_get_environ(cli);

    guint num_variables = g_strv_length((gchar**)environment);
    gchar** result = g_new0(gchar*, num_variables + 1);

    for (guint i = 0; i < num_variables; ++i) {
        result[i] = g_strdup(environment[i]);
    }
    return result;
}

void ConnectSignal(GtkWidget* widget, const char* signal_name, GCallback callback, gpointer user_data) {
    g_signal_connect(widget, signal_name, callback, user_data);
}

void ConnectVteSignals(GtkWidget* widget, GtkWidget* window) {
    ConnectSignal(widget, "child-exited", G_CALLBACK(ChildExited), window);
    ConnectSignal(widget, "window-title-changed", G_CALLBACK(WindowTitleChanged), window);
    ConnectSignal(widget, "button-press-event", G_CALLBACK(ButtonPressEvent), NULL);
    ConnectSignal(window, "delete-event", G_CALLBACK(ConfirmExit), NULL);
}

void SpawnVteTerminal(GApplicationCommandLine* cli, GtkWidget* window, GtkWidget* widget) {
    GVariantDict* options = g_application_command_line_get_options_dict(cli);
    const gchar* command = NULL;
    g_variant_dict_lookup(options, "cmd", "&s", &command);
    gchar** environment = GetEnviroment(cli);
    gchar** cmd;
    gchar* cmdline = NULL;
    cmd = command ?
        (gchar*[]) {"/bin/sh", cmdline = g_strdup(command), NULL} :
        (gchar*[]) {cmdline = g_strdup(g_application_command_line_getenv(cli, "SHELL")), NULL};

    ConnectVteSignals(widget, window);

    vte_terminal_set_word_char_exceptions(VTE_TERMINAL(widget), "-./?%&_=+@~:");
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(widget), -1);
    vte_terminal_set_scroll_on_output(VTE_TERMINAL(widget), TRUE);
    vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(widget), TRUE);
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(widget), TRUE);
    vte_terminal_set_bold_is_bright(VTE_TERMINAL(widget), TRUE);
    vte_terminal_set_audible_bell(VTE_TERMINAL(widget), TRUE);
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(widget), TRUE);

    vte_terminal_spawn_async(VTE_TERMINAL(widget),
        VTE_PTY_DEFAULT,
        g_application_command_line_get_cwd(cli),
        cmd,
        environment,
        0,
        NULL,
        NULL,
        NULL,
        -1,
        NULL,
        ChildReady,
        window);

    g_strfreev(environment);
    g_free(cmdline);
}

void CloseWindow(void) {
    exit(0);
}

GtkWidget* FileMenuHelper(const gchar* iconPath, const gchar* label, const gchar* shortcut, GCallback callback) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(box, TRUE);

    GtkWidget* icon = gtk_image_new_from_file(iconPath);
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);

    GtkWidget* spacing = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(spacing), "    ");
    gtk_box_pack_start(GTK_BOX(box), spacing, FALSE, FALSE, 0);

    GtkWidget* titleLabel = gtk_label_new(label);
    gtk_box_pack_start(GTK_BOX(box), titleLabel, TRUE, TRUE, 0);
    gtk_label_set_xalign(GTK_LABEL(titleLabel), 0.0);

    GtkWidget* shortcutLabel = gtk_label_new(shortcut);
    PangoAttrList* shortcutAttrList = pango_attr_list_new();
    PangoAttribute* shortcutAttr = pango_attr_foreground_new(128 * G_MAXUINT16 / 255, 128 * G_MAXUINT16 / 255, 128 * G_MAXUINT16 / 255);
    pango_attr_list_insert(shortcutAttrList, shortcutAttr);
    gtk_label_set_attributes(GTK_LABEL(shortcutLabel), shortcutAttrList);
    gtk_box_pack_end(GTK_BOX(box), shortcutLabel, FALSE, FALSE, 0);

    GtkWidget* menuItem = gtk_menu_item_new();
    gtk_container_add(GTK_CONTAINER(menuItem), box);
    g_signal_connect(menuItem, "activate", callback, NULL);

    return menuItem;
}

GtkWidget* FileMenu() {
    GtkWidget* file_menu = gtk_menu_new();

    GtkWidget* new_window_item = FileMenuHelper("/usr/share/icons/hicolor/24x24/apps/window-new.svg", "New Window", "Shift+Ctrl+N", G_CALLBACK(NewWindow));
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_window_item);

    GtkWidget* new_tab_item = FileMenuHelper("/usr/share/icons/hicolor/24x24/apps/tab-new.svg", "New Tab", "Shift+Ctrl+T", G_CALLBACK(NewTab));
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_tab_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());

    GtkWidget* close_tab_item = FileMenuHelper("/usr/share/icons/hicolor/24x24/apps/tab-close.svg", "Close Tab", "Shift+Ctrl+W", G_CALLBACK(CloseTab));
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), close_tab_item);

    GtkWidget* close_window_item = FileMenuHelper("/usr/share/icons/hicolor/24x24/apps/window-close.svg", "Close Window", "Shift+Ctrl+Q", G_CALLBACK(CloseWindow));
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), close_window_item);

    gtk_widget_show_all(file_menu);

    return file_menu;
}

void ZoomIn(void) {
    g_print("ZoomIn\n");
}

void ZoomOut(void) {
    g_print("ZoomOut\n");
}

void ZoomReset(void) {
    g_print("ZoomReset\n");
}

void palette_selected(void)
{

}

void color_selected(void)
{

}

void OpenFontDialog(GtkWidget *widget, gpointer user_data) {
    GtkComboBoxText *font_combo_box = GTK_COMBO_BOX_TEXT(user_data);

    GtkWidget *dialog = gtk_font_chooser_dialog_new("Select Font", GTK_WINDOW(gtk_widget_get_toplevel(widget)));

    const gchar *current_font = gtk_combo_box_text_get_active_text(font_combo_box);

    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(dialog), current_font);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gchar *selected_font = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
        GtkTreeIter iter;

        if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(font_combo_box), &iter)) {
            gchar *font_family_name;
            gtk_tree_model_get(GTK_TREE_MODEL(font_combo_box), &iter, 0, &font_family_name, -1);

            if (g_strcmp0(font_family_name, selected_font) == 0) {
                gtk_combo_box_set_active_iter(GTK_COMBO_BOX(font_combo_box), &iter);
            }
            g_free(font_family_name);
        }
        g_free(selected_font);
    }
    gtk_widget_destroy(dialog);
}

void StyleTab(GtkNotebook *notebook)
{
    GtkWidget *style_tab = gtk_label_new("Style");
    GtkWidget *style_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(style_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(style_grid), 10);

    GtkWidget *font_label = gtk_label_new("Terminal Font:");
    gtk_label_set_use_markup(GTK_LABEL(font_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(font_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), font_label, 0, 0, 1, 1);

    GtkComboBoxText *font_combo_box = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());

    GtkWidget *font_button = gtk_button_new_with_label("Monospace Regular 10");
    g_signal_connect(font_button, "clicked", G_CALLBACK(OpenFontDialog), font_combo_box);
    gtk_grid_attach(GTK_GRID(style_grid), font_button, 1, 0, 1, 1);

    GtkWidget *background_label = gtk_label_new("Background:");
    gtk_label_set_use_markup(GTK_LABEL(background_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(background_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), background_label, 0, 1, 1, 1);

    GdkRGBA black_color;
    gdk_rgba_parse(&black_color, "black");
	
    GtkWidget *background_color_button = gtk_color_button_new_with_rgba(&black_color);
    gtk_widget_set_size_request(background_color_button, 100, -1);
    gtk_grid_attach(GTK_GRID(style_grid), background_color_button, 1, 1, 1, 1);
    
    GtkWidget *foreground_label = gtk_label_new("Foreground:");
    gtk_label_set_use_markup(GTK_LABEL(foreground_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(foreground_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), foreground_label, 0, 2, 1, 1);

    GdkRGBA gray_color;
    gdk_rgba_parse(&gray_color, "gray");

    GtkWidget *foreground_color_button = gtk_color_button_new_with_rgba(&gray_color);
    gtk_widget_set_size_request(foreground_color_button, 100, -1);
    gtk_grid_attach(GTK_GRID(style_grid), foreground_color_button, 1, 2, 1, 1);

    GtkWidget *palette_label = gtk_label_new("Palette:");
    gtk_label_set_use_markup(GTK_LABEL(palette_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(palette_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), palette_label, 0, 3, 1, 1);

    GtkComboBoxText *palette_combo_box = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append(palette_combo_box, "vga", "VGA");
    gtk_combo_box_text_append(palette_combo_box, "xterm", "xterm");
    gtk_combo_box_text_append(palette_combo_box, "tango", "Tango");
    gtk_combo_box_text_append(palette_combo_box, "solarized-dark", "Solarized Dark");
    gtk_combo_box_text_append(palette_combo_box, "solarized-light", "Solarized Light");
    gtk_combo_box_text_append(palette_combo_box, "custom", "Custom");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(palette_combo_box), "vga");

    g_signal_connect(palette_combo_box, "changed", G_CALLBACK(palette_selected), NULL);
    gtk_grid_attach(GTK_GRID(style_grid), GTK_WIDGET(palette_combo_box), 1, 3, 1, 1);

    const char *color_names[] = {"Black", "Red", "Green", "Brown", "Blue", "Magenta", "Cyan", "Gray"};
    GdkRGBA colors[] = {{0, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 0, 1}, {0.6, 0.4, 0.2, 1}, {0, 0, 1, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}, {0.5, 0.5, 0.5, 1}};
    int num_colors = sizeof(color_names) / sizeof(color_names[0]);
    int color_button_width = 85;

    GtkWidget *color_button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(color_button_box), GTK_BUTTONBOX_CENTER);
    gtk_grid_attach(GTK_GRID(style_grid), color_button_box, 0, 4, num_colors, 1);

    for (int i = 0; i < num_colors; i++) {
        GtkWidget *color_button = gtk_color_button_new_with_rgba(&colors[i]);
        gtk_widget_set_size_request(color_button, color_button_width, -1);
        gtk_container_add(GTK_CONTAINER(color_button_box), color_button);
        g_signal_connect(color_button, "color-set", G_CALLBACK(color_selected), NULL);
    }

    const char *additional_color_names[] = {"Light Gray", "Bright Red", "Bright Green", "Yellow", "Bright Blue", "Bright Magenta", "Bright Cyan", "White"};
    GdkRGBA additional_colors[] = {{0.9, 0.9, 0.9, 1}, {1, 0.5, 0.5, 1}, {0.5, 1, 0.5, 1}, {1, 1, 0.8, 1}, {0.5, 0.5, 1, 1}, {1, 0.5, 1, 1}, {0.5, 1, 1, 1}, {1, 1, 1, 1}};
    int num_additional_colors = sizeof(additional_color_names) / sizeof(additional_color_names[0]);

    GtkWidget *additional_color_button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(additional_color_button_box), GTK_BUTTONBOX_CENTER);
    gtk_grid_attach(GTK_GRID(style_grid), additional_color_button_box, 0, 5, num_additional_colors, 1);

    for (int i = 0; i < num_additional_colors; i++) {
        GtkWidget *additional_color_button = gtk_color_button_new_with_rgba(&additional_colors[i]);
        gtk_widget_set_size_request(additional_color_button, color_button_width, -1);
        gtk_container_add(GTK_CONTAINER(additional_color_button_box), additional_color_button);
        g_signal_connect(additional_color_button, "color-set", G_CALLBACK(color_selected), NULL);
    }

    GtkWidget *cursor_blink_time_label = gtk_label_new("Cursor Blink Time (ms):");
    gtk_label_set_use_markup(GTK_LABEL(cursor_blink_time_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(cursor_blink_time_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), cursor_blink_time_label, 0, 6, 1, 1);

    GtkAdjustment *cursor_blink_time_adjustment = gtk_adjustment_new(500, 100, 5000, 100, 100, 0);
    GtkWidget *cursor_blink_time_spin = gtk_spin_button_new(cursor_blink_time_adjustment, 1, 0);
    gtk_grid_attach(GTK_GRID(style_grid), cursor_blink_time_spin, 1, 6, 1, 1);
    gtk_widget_set_size_request(cursor_blink_time_spin, 100, -1);
    
    GtkWidget *allow_bold_font_label = gtk_label_new("Allow Bold Font:");
    gtk_label_set_use_markup(GTK_LABEL(allow_bold_font_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(allow_bold_font_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), allow_bold_font_label, 0, 7, 1, 1);

    GtkWidget *allow_bold_font_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(allow_bold_font_check), TRUE);
    gtk_grid_attach(GTK_GRID(style_grid), allow_bold_font_check, 1, 7, 1, 1);
    
    GtkWidget *bold_is_bright_label = gtk_label_new("Bold is Bright:");
    gtk_label_set_use_markup(GTK_LABEL(bold_is_bright_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(bold_is_bright_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), bold_is_bright_label, 0, 8, 1, 1);

    GtkWidget *cursor_blink_label = gtk_label_new("Cursor Blink:");
    gtk_label_set_use_markup(GTK_LABEL(cursor_blink_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(cursor_blink_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), cursor_blink_label, 0, 9, 1, 1);

    GtkWidget *cursor_blink_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cursor_blink_check), FALSE);
    gtk_grid_attach(GTK_GRID(style_grid), cursor_blink_check, 1, 9, 1, 1);

    GtkWidget *bold_is_bright_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bold_is_bright_check), FALSE);
    gtk_grid_attach(GTK_GRID(style_grid), bold_is_bright_check, 1, 8, 1, 1);
    
    GtkWidget *cursor_style_label = gtk_label_new("Cursor Style:");
    gtk_label_set_use_markup(GTK_LABEL(cursor_style_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(cursor_style_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), cursor_style_label, 0, 10, 1, 1);

    GtkWidget *radio_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_grid_attach(GTK_GRID(style_grid), radio_button_box, 1, 10, 2, 1);

    GtkWidget *block_cursor_radio = gtk_radio_button_new_with_label(NULL, "Block");
    gtk_box_pack_start(GTK_BOX(radio_button_box), block_cursor_radio, FALSE, FALSE, 0);

    GtkWidget *underline_cursor_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(block_cursor_radio), "Underline");
    gtk_box_pack_start(GTK_BOX(radio_button_box), underline_cursor_radio, FALSE, FALSE, 0);
    gtk_grid_attach(GTK_GRID(style_grid), underline_cursor_radio, 1, 11, 1, 1);

    GtkWidget *audible_bell_label = gtk_label_new("Audible Bell:");
    gtk_label_set_use_markup(GTK_LABEL(audible_bell_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(audible_bell_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), audible_bell_label, 0, 11, 1, 1);

    GtkWidget *audible_bell_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(audible_bell_check), FALSE);
    gtk_grid_attach(GTK_GRID(style_grid), audible_bell_check, 1, 11, 1, 1);

    GtkWidget *visual_bell_label = gtk_label_new("Visual Bell:");
    gtk_label_set_use_markup(GTK_LABEL(visual_bell_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(visual_bell_label), 0);
    gtk_grid_attach(GTK_GRID(style_grid), visual_bell_label, 0, 12, 1, 1);

    GtkWidget *visual_bell_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visual_bell_check), FALSE);
    gtk_grid_attach(GTK_GRID(style_grid), visual_bell_check, 1, 12, 1, 1);
    gtk_notebook_append_page(notebook, style_grid, style_tab);
}

void DisplayTab(GtkNotebook *notebook) {
    GtkWidget *display_tab = gtk_label_new("Display");
    GtkWidget *display_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(display_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(display_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(display_grid), 10);

    GtkWidget *tab_position_label = gtk_label_new("Tab panel position:");
    gtk_grid_attach(GTK_GRID(display_grid), tab_position_label, 0, 0, 1, 1);

    GtkWidget *tab_position_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(tab_position_combo), "Top");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(tab_position_combo), "Bottom");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(tab_position_combo), "Left");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(tab_position_combo), "Right");
    gtk_combo_box_set_active(GTK_COMBO_BOX(tab_position_combo), 0);
    gtk_grid_attach(GTK_GRID(display_grid), tab_position_combo, 1, 0, 1, 1);

    GtkWidget *default_size_label = gtk_label_new("Default window size:");
    gtk_grid_attach(GTK_GRID(display_grid), default_size_label, 0, 2, 1, 1);

    GtkWidget *style_grid = gtk_grid_new();

    GtkAdjustment *width_adjustment = gtk_adjustment_new(80, 1, 1000, 1, 100, 0);
    GtkWidget *width_spin = gtk_spin_button_new(width_adjustment, 1, 0);
    gtk_widget_set_size_request(width_spin, 292, -1);
    gtk_grid_attach(GTK_GRID(style_grid), width_spin, 0, 0, 1, 1);

    GtkWidget *x_label = gtk_label_new("x");
    gtk_widget_set_halign(x_label, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(style_grid), x_label, 1, 0, 1, 1);

    GtkAdjustment *height_adjustment = gtk_adjustment_new(24, 1, 1000, 1, 100, 0);
    GtkWidget *height_spin = gtk_spin_button_new(height_adjustment, 1, 0);
    gtk_widget_set_size_request(height_spin, 100, -1);
    gtk_grid_attach(GTK_GRID(style_grid), height_spin, 2, 0, 1, 1);

    gtk_widget_set_margin_start(x_label, 6);
    gtk_widget_set_margin_end(x_label, 6);

    GtkWidget *scrollback_label = gtk_label_new("Scrollback Lines:");
    gtk_grid_attach(GTK_GRID(display_grid), scrollback_label, 0, 1, 1, 1);
    GtkAdjustment *scrollback_adjustment = gtk_adjustment_new(1000, 1, 10000, 1, 100, 0);

    GtkWidget *scrollback_spin = gtk_spin_button_new(scrollback_adjustment, 1, 0);
    gtk_widget_set_size_request(scrollback_spin, 200, -1);
    gtk_grid_attach(GTK_GRID(display_grid), scrollback_spin, 1, 1, 1, 1);

    GtkWidget *separator_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(separator_box, GTK_ALIGN_START);

    GtkWidget *space_label = gtk_label_new("");
    gtk_widget_set_hexpand(space_label, TRUE);
    gtk_box_pack_start(GTK_BOX(separator_box), space_label, FALSE, FALSE, 0);

    GtkWidget *x_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *x_space_label = gtk_label_new("  ");
    gtk_box_pack_start(GTK_BOX(x_box), x_space_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(x_box), x_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(separator_box), x_box, FALSE, FALSE, 0);
    gtk_grid_attach(GTK_GRID(style_grid), separator_box, 2, 0, 1, 1);
    gtk_widget_set_halign(scrollback_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(display_grid), style_grid, 1, 2, 1, 1);

    GtkWidget *scrollback_label2 = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(display_grid), scrollback_label2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(display_grid), style_grid, 1, 2, 1, 1);

    GtkWidget *hide_scrollbar_title = gtk_label_new("Hide Scrollbar:");
    gtk_grid_attach(GTK_GRID(display_grid), hide_scrollbar_title, 0, 3, 1, 1);

    GtkWidget *hide_scrollbar_check = gtk_check_button_new();
    gtk_grid_attach(GTK_GRID(display_grid), hide_scrollbar_check, 1, 3, 1, 1);

    GtkWidget *hide_menu_bar_title = gtk_label_new("Hide Menu Bar:");
    gtk_grid_attach(GTK_GRID(display_grid), hide_menu_bar_title, 0, 4, 1, 1);

    GtkWidget *hide_menu_bar_check = gtk_check_button_new();
    gtk_grid_attach(GTK_GRID(display_grid), hide_menu_bar_check, 1, 4, 1, 1);

    GtkWidget *hide_close_buttons_title = gtk_label_new("Hide Close Buttons:");
    gtk_grid_attach(GTK_GRID(display_grid), hide_close_buttons_title, 0, 5, 1, 1);

    GtkWidget *hide_close_buttons_check = gtk_check_button_new();
    gtk_grid_attach(GTK_GRID(display_grid), hide_close_buttons_check, 1, 5, 1, 1);

    GtkWidget *hide_mouse_pointer_title = gtk_label_new("Hide Mouse Pointer:");
    gtk_grid_attach(GTK_GRID(display_grid), hide_mouse_pointer_title, 0, 6, 1, 1);

    GtkWidget *hide_mouse_pointer_check = gtk_check_button_new();
    gtk_grid_attach(GTK_GRID(display_grid), hide_mouse_pointer_check, 1, 6, 1, 1);

    gtk_widget_set_halign(tab_position_label, GTK_ALIGN_START);
    gtk_widget_set_halign(scrollback_label, GTK_ALIGN_START);
    gtk_widget_set_halign(default_size_label, GTK_ALIGN_START);
    gtk_widget_set_halign(hide_scrollbar_title, GTK_ALIGN_START);
    gtk_widget_set_halign(hide_menu_bar_title, GTK_ALIGN_START);
    gtk_widget_set_halign(hide_close_buttons_title, GTK_ALIGN_START);
    gtk_widget_set_halign(hide_mouse_pointer_title, GTK_ALIGN_START);

    gtk_notebook_append_page(notebook, display_grid, display_tab);
}

void AdvancedTab(GtkNotebook *notebook) {
    GtkWidget *advanced_tab = gtk_label_new("Advanced");
    GtkWidget *advanced_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(advanced_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(advanced_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(advanced_grid), 10);

    GtkWidget *select_word_label = gtk_label_new("Select-by-word characters:");
    GtkWidget *characters_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(characters_entry), "-A-Za-z0-9,./?%&#:_");

    GtkWidget *disable_menu_label = gtk_label_new("Disable menu shortcut key (F10 by default):");
    GtkWidget *disable_menu_checkbox = gtk_check_button_new();

    GtkWidget *disable_alt_n_label = gtk_label_new("Disable using Alt-n for tabs and menu:");
    GtkWidget *disable_alt_n_checkbox = gtk_check_button_new();

    GtkWidget *disable_confirm_label = gtk_label_new("Disable confirmation before closing a window with multiple tabs:");
    GtkWidget *disable_confirm_checkbox = gtk_check_button_new();

    GtkWidget *widgets[] = {
        select_word_label, characters_entry,
        disable_menu_label, disable_menu_checkbox,
        disable_alt_n_label, disable_alt_n_checkbox,
        disable_confirm_label, disable_confirm_checkbox
    };

    for (int i = 0; i < G_N_ELEMENTS(widgets); i += 2) {
        gtk_widget_set_halign(widgets[i], GTK_ALIGN_START);
        gtk_widget_set_halign(widgets[i + 1], GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(advanced_grid), widgets[i], 0, i / 2, 1, 1);
        gtk_grid_attach(GTK_GRID(advanced_grid), widgets[i + 1], 1, i / 2, 1, 1);
    }
    gtk_notebook_append_page(notebook, advanced_grid, advanced_tab);
}

void ShortcutsTab(GtkNotebook *notebook) {
    GtkWidget *shortcuts_tab = gtk_label_new("Shortcuts");

    GtkWidget *shortcuts_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(shortcuts_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(shortcuts_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(shortcuts_grid), 10);
    gtk_widget_set_halign(shortcuts_grid, GTK_ALIGN_END);

    GtkWidget *labels[] = {
        gtk_label_new("New Window:"),
        gtk_label_new("New Tab:"),
        gtk_label_new("Close Tab:"),
        gtk_label_new("Close Window:"),
        gtk_label_new("Copy:"),
        gtk_label_new("Paste:"),
        gtk_label_new("Zoom In:"),
        gtk_label_new("Zoom Out:"),
        gtk_label_new("Zoom Reset:"),
        gtk_label_new("Name Tab:"),
        gtk_label_new("Previous Tab:"),
        gtk_label_new("Next Tab:"),
        gtk_label_new("Move Tab Left:"),
        gtk_label_new("Move Tab Right:"),
        gtk_label_new("Move Window Left:"),
        gtk_label_new("Move Window Right:"),
        gtk_label_new("Enter Fullscreen:"),
        gtk_label_new("Reset Window Position:"),      
    };
    
    GtkWidget *shortcut_entries[] = {
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new(),
        gtk_entry_new()
    };

    const char *default_shortcuts[] = {
        "Shift+Ctrl+N",
        "Shift+Ctrl+T",
        "Shift+Ctrl+W",
        "Shift+Ctrl+Q",
        "Shift+Ctrl+C",
        "Shift+Ctrl+V",
        "Shift+Ctrl++",
        "Shift+Ctrl+_",
        "Shift+Ctrl+)",
        "Shift+Ctrl+I",
        "Shift+Ctrl+Left",
        "Shift+Ctrl+Right",
        "Shift+Ctrl+Page Up",
        "Shift+Ctrl+Page Down",
        "Super+Page Left",  
        "Super+Page Right",  
        "Super+Page Up",  
        "Super+Page Down",  

    };

    for (int i = 0; i < G_N_ELEMENTS(labels); ++i) {
        gtk_entry_set_text(GTK_ENTRY(shortcut_entries[i]), default_shortcuts[i]);

        gtk_grid_attach(GTK_GRID(shortcuts_grid), labels[i], 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(shortcuts_grid), shortcut_entries[i], 1, i, 1, 1);
    }
    gtk_notebook_append_page(notebook, shortcuts_grid, shortcuts_tab);
}

void OkButton(void) {
    g_print("OK button clicked!\n");
}

void Preferences(GtkMenuItem *menu_item, gpointer user_data)
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Preferences");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroyed), &window);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "/usr/share/icons/hicolor/48x48/apps/illumiterm.png", NULL);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(notebook), 10);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    StyleTab(GTK_NOTEBOOK(notebook));
    DisplayTab(GTK_NOTEBOOK(notebook));
    AdvancedTab(GTK_NOTEBOOK(notebook));
    ShortcutsTab(GTK_NOTEBOOK(notebook));

    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_END);
    gtk_box_pack_end(GTK_BOX(vbox), button_box, TRUE, TRUE, 15);

    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
    gtk_container_add(GTK_CONTAINER(buttons_box), cancel_button);

    GtkWidget *ok_button = gtk_button_new_with_label("OK");
    gtk_container_add(GTK_CONTAINER(buttons_box), ok_button);
    gtk_container_add(GTK_CONTAINER(button_box), buttons_box);
    g_signal_connect(ok_button, "clicked", G_CALLBACK(OkButton), NULL);

    gtk_widget_show_all(window);
}

GtkWidget* EditMenuHelper(const gchar* iconPath, const gchar* label, const gchar* shortcut, GCallback callback) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(box, TRUE);
    
    GtkWidget *icon = gtk_image_new_from_file(iconPath);
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    
    GtkWidget *spacing = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(spacing), "    ");
    gtk_box_pack_start(GTK_BOX(box), spacing, FALSE, FALSE, 0);
    
    GtkWidget *titleLabel = gtk_label_new(label);
    gtk_box_pack_start(GTK_BOX(box), titleLabel, FALSE, FALSE, 0);
    
    GtkWidget *shortcutLabel = gtk_label_new(shortcut);
    PangoAttrList *shortcutAttrList = pango_attr_list_new();
    PangoAttribute *shortcutAttr = pango_attr_foreground_new(128 * G_MAXUINT16 / 255, 128 * G_MAXUINT16 / 255, 128 * G_MAXUINT16 / 255);
    pango_attr_list_insert(shortcutAttrList, shortcutAttr);
    gtk_label_set_attributes(GTK_LABEL(shortcutLabel), shortcutAttrList);
    gtk_box_pack_end(GTK_BOX(box), shortcutLabel, FALSE, FALSE, 0);
    
    GtkWidget *menuItem = gtk_menu_item_new();
    gtk_container_add(GTK_CONTAINER(menuItem), box);
    g_signal_connect(menuItem, "activate", callback, NULL);
    
    return menuItem;
}

GtkWidget* EditMenu() {
    GtkWidget *edit_menu = gtk_menu_new();

    GtkWidget *copy_item = EditMenuHelper("/usr/share/icons/hicolor/24x24/apps/edit-copy.svg", "Copy", "Shift+Ctrl+C", G_CALLBACK(Copy));
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), copy_item);

    GtkWidget *paste_item = EditMenuHelper("/usr/share/icons/hicolor/24x24/apps/edit-paste.svg", "Paste", "Shift+Ctrl+V", G_CALLBACK(Paste));
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), paste_item);

    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), separator);

    GtkWidget *clear_scrollback_item = EditMenuHelper("/usr/share/icons/hicolor/24x24/apps/edit-clear.svg", "Clear Scrollback", "", G_CALLBACK(ClearScrollback));
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), clear_scrollback_item);

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), separator);

    GtkWidget *zoom_in_item = EditMenuHelper("/usr/share/icons/hicolor/24x24/apps/zoom-in.svg", "Zoom In", "Shift+Ctrl++", G_CALLBACK(ZoomIn));
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), zoom_in_item);

    GtkWidget *zoom_out_item = EditMenuHelper("/usr/share/icons/hicolor/24x24/apps/zoom-out.svg", "Zoom Out", "Shift+Ctrl+_", G_CALLBACK(ZoomOut));
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), zoom_out_item);

    GtkWidget *reset_item = EditMenuHelper("/usr/share/icons/hicolor/24x24/apps/zoom-original.svg", "Zoom Reset", "Shift+Ctrl+)", G_CALLBACK(ZoomReset));
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), reset_item);

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), separator);

    GtkWidget *preferences_item = EditMenuHelper("/usr/share/icons/hicolor/24x24/apps/configure.svg", "Preferences", "", G_CALLBACK(Preferences));
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), preferences_item);

    gtk_widget_show_all(copy_item);

    return edit_menu;
}

GtkWidget* TabsMenuHelper(const gchar *icon_path, const gchar *label_text, const gchar *shortcut_text, GCallback callback) {
    GtkWidget *menu_item = gtk_menu_item_new();

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *icon = gtk_image_new_from_file(icon_path);
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);

    GtkWidget *spacer = gtk_label_new("    ");
    gtk_box_pack_start(GTK_BOX(box), spacer, FALSE, FALSE, 0);

    GtkWidget *label = gtk_label_new(label_text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

    GtkWidget *shortcut = gtk_label_new(shortcut_text);
    PangoAttrList *attr_list = pango_attr_list_new();
    PangoAttribute *attr = pango_attr_foreground_new(128 * G_MAXUINT16 / 255, 128 * G_MAXUINT16 / 255, 128 * G_MAXUINT16 / 255);
    pango_attr_list_insert(attr_list, attr);
    gtk_label_set_attributes(GTK_LABEL(shortcut), attr_list);
    gtk_box_pack_end(GTK_BOX(box), shortcut, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(menu_item), box);
    g_signal_connect(menu_item, "activate", callback, NULL);

    return menu_item;
}

GtkWidget* TabsMenu() {
    GtkWidget *tabs_menu = gtk_menu_new();

    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(tabs_menu), separator);

    GtkWidget *name_tab = TabsMenuHelper("/usr/share/icons/hicolor/24x24/apps/edit.svg", "Name Tab", "Shift+Ctrl+I", G_CALLBACK(NameTab));
    gtk_menu_shell_append(GTK_MENU_SHELL(tabs_menu), name_tab);

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(tabs_menu), separator);

    GtkWidget *previous_tab = TabsMenuHelper("/usr/share/icons/hicolor/24x24/apps/go-previous.svg", "Previous Tab", "Shift+Ctrl+Left", G_CALLBACK(PreviousTab));
    gtk_menu_shell_append(GTK_MENU_SHELL(tabs_menu), previous_tab);

    GtkWidget *next_tab = TabsMenuHelper("/usr/share/icons/hicolor/24x24/apps/go-next.svg", "Next Tab", "Shift+Ctrl+Right", G_CALLBACK(NextTab));
    gtk_menu_shell_append(GTK_MENU_SHELL(tabs_menu), next_tab);

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(tabs_menu), separator);

    GtkWidget *move_tab_left = TabsMenuHelper("/usr/share/icons/hicolor/24x24/apps/go-up.svg", "Move Tab Left", "Shift+Ctrl+Page Up", G_CALLBACK(MoveTabLeft));
    gtk_menu_shell_append(GTK_MENU_SHELL(tabs_menu), move_tab_left);

    GtkWidget *move_tab_right = TabsMenuHelper("/usr/share/icons/hicolor/24x24/apps/go-down.svg", "Move Tab Right", "Shift+Ctrl+Page Down", G_CALLBACK(MoveTabRight));
    gtk_menu_shell_append(GTK_MENU_SHELL(tabs_menu), move_tab_right);

    return tabs_menu;
}

GtkWidget* PositionMenuHelper(const gchar *icon_path, const gchar *label_text, const gchar *shortcut_text, GCallback callback) {
    GtkWidget *menu_item = gtk_menu_item_new();

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *icon = gtk_image_new_from_file(icon_path);
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);

    GtkWidget *spacer = gtk_label_new("    ");
    gtk_box_pack_start(GTK_BOX(box), spacer, FALSE, FALSE, 0);

    GtkWidget *label = gtk_label_new(label_text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

    GtkWidget *shortcut = gtk_label_new(shortcut_text);
    PangoAttrList *attr_list = pango_attr_list_new();
    PangoAttribute *attr = pango_attr_foreground_new(128 * G_MAXUINT16 / 255, 128 * G_MAXUINT16 / 255, 128 * G_MAXUINT16 / 255);
    pango_attr_list_insert(attr_list, attr);
    gtk_label_set_attributes(GTK_LABEL(shortcut), attr_list);
    gtk_box_pack_end(GTK_BOX(box), shortcut, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(menu_item), box);
    g_signal_connect(menu_item, "activate", callback, NULL);

    return menu_item;
}

void MoveWindowLeft(void) {
    g_print("MoveWindowLeft\n");
}

void MoveWindowRight(void) {
    g_print("MoveWindowRight\n");
}

void EnterFullscreen(void) {
    g_print("EnterFullscreen\n");
}

void ResetWindowPosition(void) {
    g_print("ResetWindowPosition\n");
}

GtkWidget* PositionMenu() {
    GtkWidget *position_menu = gtk_menu_new();

    GtkWidget *previous_tab = PositionMenuHelper("/usr/share/icons/hicolor/24x24/apps/go-previous.svg", "Move Window Left", "Super+Left", G_CALLBACK(MoveWindowLeft));
    gtk_menu_shell_append(GTK_MENU_SHELL(position_menu), previous_tab);

    GtkWidget *next_tab = PositionMenuHelper("/usr/share/icons/hicolor/24x24/apps/go-next.svg", "Move Window Right", "Super+Right", G_CALLBACK(MoveWindowRight));
    gtk_menu_shell_append(GTK_MENU_SHELL(position_menu), next_tab);
    
    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(position_menu), separator);

    GtkWidget *move_tab_left = PositionMenuHelper("/usr/share/icons/hicolor/24x24/apps/go-up.svg", "Enter Fullscreen ", "Super+Page Up", G_CALLBACK(EnterFullscreen));
    gtk_menu_shell_append(GTK_MENU_SHELL(position_menu), move_tab_left);

    GtkWidget *move_tab_right = PositionMenuHelper("/usr/share/icons/hicolor/24x24/apps/go-down.svg", "Reset Window Position", "Super+Page Down", G_CALLBACK(ResetWindowPosition));
    gtk_menu_shell_append(GTK_MENU_SHELL(position_menu), move_tab_right);

    return position_menu;
}

void AboutWindow(GtkWindow* parent) {
    GtkAboutDialog* about_dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    GdkPixbuf* icon = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/48x48/apps/illumiterm.png", NULL);
    gtk_window_set_icon(GTK_WINDOW(about_dialog), icon);
    g_object_unref(icon);

    gtk_window_set_position(GTK_WINDOW(about_dialog), GTK_WIN_POS_NONE);
    gtk_widget_show_all(GTK_WIDGET(about_dialog));

    gtk_about_dialog_set_program_name(about_dialog, "IllumiTerm");
    gtk_about_dialog_set_version(about_dialog, "1.0.0");
    gtk_about_dialog_set_comments(about_dialog, "G.H.S");
    gtk_about_dialog_set_website(about_dialog, "https://www.illumiterm.com");
    gtk_about_dialog_set_license(about_dialog, "This program is free software; you can redistribute it and/or\n"
                                               "modify it under the terms of the GNU General Public License\n"
                                               "as published by the Free Software Foundation; either version 2\n"
                                               "of the License, or (at your option) any later version.\n\n"
                                               "This program is distributed in the hope that it will be useful,\n"
                                               "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                                               "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                                               "GNU General Public License for more details.\n\n"
                                               "You should have received a copy of the GNU General Public License\n"
                                               "along with this program; if not, write to the Free Software\n"
                                               "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.");

    GdkPixbuf* logo_pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/96x96/apps/about.png", NULL);
    gtk_about_dialog_set_logo(about_dialog, logo_pixbuf);
    g_object_unref(logo_pixbuf);
    
    const gchar* authors[] = {"Elijah Gordon", "<a href=\"mailto:braindisassemblue@gmail.com\">braindisassemblue@gmail.com</a>", NULL};

    gtk_about_dialog_set_authors(about_dialog, authors);
    gtk_about_dialog_set_copyright(about_dialog, "Copyright © 2023 Elijah Gordon");
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(GTK_WIDGET(about_dialog));
}

void About(GtkMenuItem* menuitem, gpointer user_data) {
    GtkWindow* window = GTK_WINDOW(user_data);
    AboutWindow(window);
}

GtkWidget* AboutMenu() {
    GtkWidget* about_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(about_box, TRUE);

    GtkWidget* about_icon = gtk_image_new_from_file("/usr/share/icons/hicolor/24x24/apps/help-about.svg");
    GtkWidget* icon_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(icon_box), about_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(icon_box), gtk_label_new("    "), FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(about_box), icon_box, FALSE, FALSE, 0);
    
    GtkWidget* about_label = gtk_label_new("About");
    gtk_label_set_xalign(GTK_LABEL(about_label), 0.0);
    gtk_box_pack_start(GTK_BOX(about_box), about_label, TRUE, TRUE, 0);

    GtkWidget* about_menu_item = gtk_menu_item_new();
    gtk_container_add(GTK_CONTAINER(about_menu_item), about_box);
    g_signal_connect(about_menu_item, "activate", G_CALLBACK(About), NULL);

    return about_menu_item;
}

GtkWidget* HelpMenu() {
    GtkWidget* help_menu = gtk_menu_new();
    GtkWidget* about_menu_item = AboutMenu();

    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_menu_item);

    return help_menu;
}

GtkWidget* CreateMenu() {
    GtkWidget *menu_bar = gtk_menu_bar_new();

    GtkWidget *file_menu_item = gtk_menu_item_new_with_label("File");
    GtkWidget *file_menu = FileMenu();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_menu_item);

    GtkWidget *edit_menu_item = gtk_menu_item_new_with_label("Edit");
    GtkWidget *edit_menu = EditMenu();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_menu_item), edit_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit_menu_item);

    GtkWidget *tabs_menu_item = gtk_menu_item_new_with_label("Tabs");
    GtkWidget *tabs_menu = TabsMenu();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tabs_menu_item), tabs_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), tabs_menu_item);

    GtkWidget *position_menu_item = gtk_menu_item_new_with_label("Position");
    GtkWidget *position_menu = PositionMenu();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(position_menu_item), position_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), position_menu_item);
    
    GtkWidget *help_menu_item = gtk_menu_item_new_with_label("Help");
    GtkWidget *help_menu = HelpMenu();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_menu_item);

    return menu_bar;
}

void SetNotebookShowTabs(GtkWidget* notebook) {
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
}

GtkWidget* CreateNotebook(GtkWidget* widget) {
    GtkWidget* notebook = gtk_notebook_new();
    SetNotebookShowTabs(notebook);

    GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), widget);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window), 200);

    GtkWidget* tab_label = gtk_label_new("Tab");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, tab_label);
    gtk_widget_show_all(notebook);

    return notebook;
}

GtkWidget* CreateWindow(GtkWidget* menu_bar, GtkWidget* notebook) {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);

    gtk_container_add(GTK_CONTAINER(scrolled_window), notebook);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "/usr/share/icons/hicolor/48x48/apps/illumiterm.png", NULL);
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_window_set_title(GTK_WINDOW(window), NULL);
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 460);
    gtk_window_set_icon_name(GTK_WINDOW(window), NULL);
    gtk_widget_show_all(window);

    return window;
}

void CommandLine(GApplication *application, GApplicationCommandLine *cli, gpointer data) {
    GtkWidget *widget = vte_terminal_new();
    GtkWidget *menu_bar = CreateMenu();
    GtkWidget *notebook = CreateNotebook(widget);
    GtkWidget *window = CreateWindow(menu_bar, notebook);

    g_application_hold(application);
    g_object_set_data_full(G_OBJECT(cli), "application", application, (GDestroyNotify) g_application_release);
    g_object_set_data_full(G_OBJECT(window), "cli", cli, NULL);
    g_object_ref(cli);

    SpawnVteTerminal(cli, window, widget);
}

void ConnectSignals(GtkApplication *application) {
    g_signal_connect(application, "command-line", G_CALLBACK(CommandLine), NULL);
}

int RunApp(int argc, char **argv) {
    GtkApplication *application = gtk_application_new("slck.illumiterm", G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_SEND_ENVIRONMENT); 

    ConnectSignals(application);
    
    int status = g_application_run(G_APPLICATION(application), argc, argv);
    
    g_object_unref(application);
    
    return status;
}

int main(int argc, char **argv) {
    int status = RunApp(argc, argv);

    return status;
}

// gcc -O2 -Wall $(pkg-config --cflags vte-2.91) $(pkg-config --cflags gtk+-3.0) illumiterm.c -o illumiterm $(pkg-config --libs vte-2.91) $(pkg-config --libs gtk+-3.0)
