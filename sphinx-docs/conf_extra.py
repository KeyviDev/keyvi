from os.path import join, dirname
import sphinx_wagtail_theme

html_theme = "sphinx_wagtail_theme"
html_theme_path = [sphinx_wagtail_theme.get_html_theme_path()]

html_static_path = [join(dirname(__file__), "_static")]

html_theme_options = dict(
    project_name= "keyvi",
    logo= "logo-white.png",
    logo_alt = "keyvi",
    github_url = "https://github.com/KeyviDev/keyvi/tree/master/sphinx-docs/",
    footer_links = ",".join([
        "Github|https://github.com/KeyviDev/keyvi",
        "Pypi|https://pypi.org/project/keyvi/",
    ]),
 )

html_show_copyright = False
html_last_updated_fmt = "%b %d, %Y"

html_css_files = ["custom.css"]