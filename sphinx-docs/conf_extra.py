from __future__ import annotations

from pathlib import Path

import sphinx_wagtail_theme

html_theme = "sphinx_wagtail_theme"
html_theme_path = [sphinx_wagtail_theme.get_html_theme_path()]

html_static_path = [str(Path(__file__).parent / "_static")]

# (font awesome icon, label, url), rendered top right of the header bar,
# on narrow screens only the icon is shown (see custom.css)
_header_links = [
    ("fab fa-python", "PyPI", "https://pypi.org/project/keyvi/"),
    ("fab fa-rust", "crates.io", "https://crates.io/crates/keyvi"),
    ("fab fa-github", "GitHub", "https://github.com/KeyviDev/keyvi"),
]

html_theme_options = {
    "project_name": "keyvi",
    "logo": "logo-white.png",
    "logo_alt": "keyvi",
    "github_url": "https://github.com/KeyviDev/keyvi/tree/master/sphinx-docs/",
    # the theme expects "text|url,text|url", so text and url must not contain "," or "|"
    "header_links": ",".join(
        f'<i class="{icon} fa-lg" title="keyvi on {label}"></i>'
        f' <span class="link-text">{label}</span>|{url}'
        for icon, label, url in _header_links
    ),
    "footer_links": (
        "Github|https://github.com/KeyviDev/keyvi,"
        "Pypi|https://pypi.org/project/keyvi/,"
        "crates.io|https://crates.io/crates/keyvi,"
        "Sponsor|https://github.com/sponsors/hendrikmuhs,"
        "Buy me a coffee|https://buymeacoffee.com/hendrikmuhs"
    ),
}

html_show_copyright = False
html_last_updated_fmt = "%b %d, %Y"

html_css_files = ["custom.css"]
