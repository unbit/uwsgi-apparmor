#include <uwsgi.h>
#include <sys/apparmor.h>

#if !defined(UWSGI_PLUGIN_API) || UWSGI_PLUGIN_API < 1
#error "this plugin requires version 1 of the uWSGI api"
#endif

static struct uwsgi_apparmor {
	char *profile;
} uapparmor;

static struct uwsgi_option apparmor_options[] = {
	{"apparmor-profile", required_argument, 0, "set apparmor profile before privileges drop", uwsgi_opt_set_str, &uapparmor.profile, 0},
	UWSGI_END_OF_OPTIONS
};

static void vassal_apply_apparmor(struct uwsgi_instance *ui) {
}

static void apply_apparmor_before_privileges_drop() {
	if (!uapparmor.profile) return;
	if (aa_change_profile(uapparmor.profile)) {
		uwsgi_error("apply_apparmor_before_privileges_drop()/aa_change_profile()");
		exit(1);
	}
}

struct uwsgi_plugin apparmor_plugin = {
	.name = "apparmor",
	.options = apparmor_options,
	.before_privileges_drop = apply_apparmor_before_privileges_drop,
	.vassal_before_exec = vassal_apply_apparmor,
};