from os.path import join, dirname, abspath
import alabaster

html_theme = "alabaster"

html_static_path = [join(dirname(__file__), "_static")]

html_theme_options = {
    "logo": "logo.png",
    "logo_name": True,
    "logo_text_align": "center",
    "github_user": "keyvidev",
    "github_repo": "keyvi",
    "github_type": "star"
}
