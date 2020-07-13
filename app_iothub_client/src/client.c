#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "azure_c_shared_utility/shared_util_options.h"
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/compile.h>
#include <mruby/data.h>
#include <mruby/dump.h>
#include <mruby/numeric.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>
#include "pinkit.h"
#include "client.h"

#ifdef _MSC_VER
extern int sprintf_s(char* dst, size_t dstSizeInBytes, const char* format, ...);
#endif // _MSC_VER

extern char* connectionString;

extern bool g_use_proxy;
extern HTTP_PROXY_OPTIONS g_proxy_options;

void iothub_client_init()
{
#if 0
	char *set_cs_argv[2] = {
		"set_cs",
		"[device connection string]"
	};
	set_cs_main(2, set_cs_argv);

	char *set_proxy_argv[2] = {
		"set_proxy",
		"example.com:8080"
	};
	set_proxy_main(2, set_proxy_argv);
#endif
}

int run_mruby_code(int argc, char **argv, const uint8_t *code, const char *cmdline)
{
	mrb_state *mrb;
	struct RProc *n;
	struct mrb_irep *irep;
	mrb_value ARGV;
	mrbc_context *c;
	mrb_value v;
	mrb_sym zero_sym;
	int result = 0;

	/* mrubyの初期化 */
	mrb = mrb_open();
	if (mrb == NULL)
		return -1;

	int ai = mrb_gc_arena_save(mrb);
	ARGV = mrb_ary_new_capa(mrb, argc);
	for (int i = 0; i < argc; i++) {
		mrb_ary_push(mrb, ARGV, mrb_str_new_cstr(mrb, argv[i]));
	}
	mrb_define_global_const(mrb, "ARGV", ARGV);

	c = mrbc_context_new(mrb);
	c->dump_result = TRUE;

	/* Set $0 */
	zero_sym = mrb_intern_lit(mrb, "$0");
	mrbc_filename(mrb, c, cmdline);
	mrb_gv_set(mrb, zero_sym, mrb_str_new_cstr(mrb, cmdline));

	irep = mrb_read_irep(mrb, code);
	n = mrb_proc_new(mrb, irep);
	v = mrb_run(mrb, n, mrb_nil_value());

	mrb_gc_arena_restore(mrb, ai);
	mrbc_context_free(mrb, c);
	if (mrb->exc) {
		if (!mrb_undef_p(v)) {
			mrb_print_error(mrb);
		}
		result = -1;
	}

	mrb_close(mrb);
	return result;
}

int iothub_client_main(int argc, char **argv)
{
	extern const uint8_t main_code[];

	if (connectionString == NULL) {
		printf("Not set connection string, use dps_csgen or set_cs.\n");
		return 0;
	}

	if (argc < 2) {
		return run_mruby_code(argc - 1, &argv[1], main_code, "iothub");
	}

	if (strcmp(argv[1], "http") == 0) {
		return run_mruby_code(argc - 1, &argv[1], main_code, "iothub");
	}
	else if (strcmp(argv[1], "mqtt") == 0) {
		return run_mruby_code(argc - 1, &argv[1], main_code, "iothub");
	}
	else if (strcmp(argv[1], "mqttows") == 0) {
		return run_mruby_code(argc - 1, &argv[1], main_code, "iothub");
	}

	printf("%s [http|mqtt|mqttows] \n", argv[0]);
	return 0;
}

mrb_value mrb_kernel_sleep(mrb_state *mrb, mrb_value self);

mrb_value mrb_pinkit_wind_speed(mrb_state *mrb, mrb_value self)
{
	double windSpeed = 
		sqrt(pinkit.accel.X * pinkit.accel.X
			+ pinkit.accel.Y * pinkit.accel.Y
			+ pinkit.accel.Z * pinkit.accel.Z);

	return mrb_float_value(mrb, (mrb_float)windSpeed);
}

mrb_value mrb_pinkit_temperature(mrb_state *mrb, mrb_value self)
{
	return mrb_float_value(mrb, (mrb_float)pinkit.temperature);
}

mrb_value mrb_pinkit_humidity(mrb_state *mrb, mrb_value self)
{
	return mrb_float_value(mrb, (mrb_float)pinkit.humidity);
}

mrb_value mrb_pinkit_get_led(mrb_state *mrb, mrb_value self)
{
	return mrb_bool_value(pinkit.ledOn != 0);
}

mrb_value mrb_pinkit_set_led(mrb_state *mrb, mrb_value self)
{
	mrb_bool ledOn = pinkit.ledOn;

	mrb_get_args(mrb, "b", &ledOn);

	pinkit.ledOn = ledOn ? 1 : 0;

	return mrb_bool_value(pinkit.ledOn != 0);
}

void mrb_mruby_others_gem_init(mrb_state *mrb)
{
	mrb_define_method(mrb, mrb->kernel_module, "sleep", mrb_kernel_sleep, MRB_ARGS_REQ(1));

	struct RClass *_module_pinkit;

	_module_pinkit = mrb_define_module(mrb, "PinKit");

	mrb_define_module_function(mrb, _module_pinkit, "wind_speed", mrb_pinkit_wind_speed, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, _module_pinkit, "temperature", mrb_pinkit_temperature, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, _module_pinkit, "humidity", mrb_pinkit_humidity, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, _module_pinkit, "led", mrb_pinkit_get_led, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, _module_pinkit, "led=", mrb_pinkit_set_led, MRB_ARGS_REQ(1));
}

void mrb_mruby_others_gem_final(mrb_state *mrb)
{
}
