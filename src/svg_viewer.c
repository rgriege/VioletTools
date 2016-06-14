#include <stdlib.h>

#include <windows.h>
#include <shobjidl.h>

#include "violet/gui/gui.h"
#include "violet/gui/svg.h"
#include "violet/utility/hash.h"
#include "violet/utility/log.h"
#include "violet/utility/time.h"
#include "violet/serialization/stream.h"
#include "violet/structures/array_map.h"

static b8 _get_file_to_open(char ** file_name)
{
	b8 retval = false;

	if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
		goto out;

	IFileOpenDialog * dialog;
	if (!SUCCEEDED(CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_ALL, &IID_IFileOpenDialog, &dialog)))
		goto err_init;

	if (!SUCCEEDED(dialog->lpVtbl->Show(dialog, NULL)))
		goto err_dlg;

	IShellItem * item;
	if (!SUCCEEDED(dialog->lpVtbl->GetResult(dialog, &item)))
		goto err_dlg;

	PWSTR psz_file_path;
	if (!SUCCEEDED(item->lpVtbl->GetDisplayName(item, SIGDN_FILESYSPATH, &psz_file_path)))
		goto err_itm;

	CoTaskMemFree(psz_file_path);
	*file_name = malloc(64);
	wcstombs(*file_name, psz_file_path, 64);
	retval = true;

err_itm:
	item->lpVtbl->Release(item);
err_dlg:
	dialog->lpVtbl->Release(dialog);
err_init:
	CoUninitialize();
out:
	return retval;
}

int main(int argc, char ** argv)
{
	int retval = 1;

	char * file_name;
	if (argc > 2)
		goto out;
	else if (argc == 1)
	{
		if (!_get_file_to_open(&file_name))
			goto out;
	}
	else
		file_name = argv[1];

	FILE * log_file = fopen("log.txt", "w");
	if (!log_file)
	{
		printf("failed to open log file\n");
		goto out;
	}

	log_init();

	ostream log_file_stream = { .type = FILE_STREAM, .file = log_file };
	log_add_stream(&log_file_stream);

	array_map text_hooks;
	array_map_init(&text_hooks, sizeof(u32), sizeof(void(*)(void*,const char *,char*)));

	array_map btn_hooks;
	array_map_init(&btn_hooks, sizeof(u32), sizeof(void(*)(void*,const char *)));

	vlt_svg svg;
	if (!vlt_svg_init_from_file(&svg, file_name))
    {
        log_write("Failed to open svg '%s'", file_name);
		goto err_svg;
    }

	vlt_gui * gui = vlt_gui_create();
	if (!vlt_gui_init_window(gui, svg.window.top_left.x, svg.window.top_left.y,
	                         svg.window.bottom_right.x, svg.window.bottom_right.y,
	                         g_white, "svgViewer"))
		goto err_gui;


	const u32 target_frame_milli = 100;
	vlt_time start, end;
	while(vlt_gui_begin_frame(gui))
	{
		vlt_get_time(&start);
		vlt_svg_render(gui, &svg, NULL, &text_hooks, &btn_hooks);
		vlt_gui_end_frame(gui);
		vlt_get_time(&end);
		const u32 frame_milli = vlt_diff_milli(&start, &end);
		if (frame_milli < target_frame_milli)
			vlt_sleep_milli(target_frame_milli - frame_milli);
		else
			log_write("long frame: %ums", frame_milli);
	}
	retval = 0;
	vlt_gui_destroy_window(gui);

err_gui:
	vlt_gui_destroy(gui);
	vlt_svg_destroy(&svg);
err_svg:
	array_map_destroy(&text_hooks);
	array_map_destroy(&btn_hooks);
	log_remove_stream(&log_file_stream);
	fclose(log_file);
	log_destroy();
out:
	return retval;
}

