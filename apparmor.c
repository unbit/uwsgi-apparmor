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
	{"apparmor-profile", required_argument, 0, "set apparmor profile before privileges drop", uwsgi_opt_set_str, &uapparmor.profile, 0},
#if UWSGI_PLUGIN_API > 1
	{"emperor-apparmor-attr", required_argument, 0, "set vassal apparmor profile using the specified attr", uwsgi_opt_set_str, &uapparmor.emperor_apparmor_attr, 0},
#endif
	{"emperor-apparmor", required_argument, 0, "set vassals apparmor profile", uwsgi_opt_set_str, &uapparmor.emperor_apparmor, 0},
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
