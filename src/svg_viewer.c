#include <stdlib.h>

#include "violet/gui/gui.h"
#include "violet/gui/svg.h"
#include "violet/math/aabb.h"
#include "violet/math/v2.h"
#include "violet/utility/hash.h"
#include "violet/utility/log.h"
#include "violet/utility/time.h"
#include "violet/serialization/stream.h"
#include "violet/structures/array.h"
#include "violet/structures/array_map.h"

int main(int argc, const char ** argv)
{
	int retval = 1;

    if (argc != 2)
        goto out;

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
	if (!vlt_svg_init_from_file(&svg, argv[1]))
    {
        log_write("Failed to open svg '%s'", argv[1]);
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

