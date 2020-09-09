#include <uwsgi.h>
#include <sys/apparmor.h>

#if !defined(UWSGI_PLUGIN_API) || UWSGI_PLUGIN_API < 1
#error "this plugin requires version 1 of the uWSGI api"
#endif

static struct uwsgi_apparmor {
	char *profile;
#if UWSGI_PLUGIN_API > 1
	char *emperor_apparmor_attr;
#endif
	char *emperor_apparmor;
} uapparmor;

static struct uwsgi_option apparmor_options[] = {
	{
		.name     = "apparmor-profile",
		.type     = required_argument,
		.shortcut = 0,
		.help     = "set apparmor profile before privileges drop",
		.func     = uwsgi_opt_set_str,
		.data     = &uapparmor.profile,
		.flags    = 0
	},
#if UWSGI_PLUGIN_API > 1
	{
		.name     = "emperor-apparmor-attr",
		.type     = required_argument,
		.shortcut = 0,
		.help     = "set vassal apparmor profile using the specified attr",
		.func     = uwsgi_opt_set_str,
		.data     = &uapparmor.emperor_apparmor_attr,
		.flags    = 0
	},
#endif
	{
		.name     = "emperor-apparmor",
		.type     = required_argument,
		.shortcut = 0,
		.help     = "set vassals apparmor profile",
		.func     = uwsgi_opt_set_str,
		.data     = &uapparmor.emperor_apparmor,
		.flags    = 0
	},
	UWSGI_END_OF_OPTIONS
};

#if UWSGI_PLUGIN_API > 1
static void vassal_apply_apparmor(struct uwsgi_instance *ui, char **argv) {
#else
static void vassal_apply_apparmor(struct uwsgi_instance *ui) {
#endif
	char *profile = uapparmor.emperor_apparmor;
#if UWSGI_PLUGIN_API > 1
	if (uapparmor.emperor_apparmor_attr) {
		profile = vassal_attr_get(ui, uapparmor.emperor_apparmor_attr);
	}
#endif
	if (!profile) return;

	uwsgi_log("[apparmor] setting profile \"%s\" ...\n", profile);
	if (aa_change_profile(profile)) {
                uwsgi_error("vassal_apply_apparmor()/aa_change_profile()");
                exit(1);
        }
}

static void apply_apparmor_before_privileges_drop() {
	if (!uapparmor.profile) return;
	uwsgi_log("[apparmor] setting profile \"%s\" ...\n", uapparmor.profile);
	if (aa_change_profile(uapparmor.profile)) {
		uwsgi_error("apply_apparmor_before_privileges_drop()/aa_change_profile()");
		exit(1);
	}
}

static int apparmor_hook(char *arg) {
	int ret = aa_change_profile(arg);
	if (ret) {
		uwsgi_error("apparmor_hook()/aa_change_profile()");
	}
	return ret;
}

static void apparmor_register() {
	uwsgi_register_hook("apparmor", apparmor_hook);
}

struct uwsgi_plugin apparmor_plugin = {
	.name = "apparmor",
	.on_load = apparmor_register,
	.options = apparmor_options,
	.before_privileges_drop = apply_apparmor_before_privileges_drop,
	.vassal_before_exec = vassal_apply_apparmor,
};
