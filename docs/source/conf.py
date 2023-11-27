# Configuration file for the Sphinx documentation builder.

# -- Project information

project = 'PAL'
copyright = '2023, Technical University of Munich'
author = 'Carlo R. Sucameli'

release = '0.1'
version = '0.1.0'

# -- General configuration

extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.doctest',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.intersphinx',
    'sphinx.ext.autosectionlabel',
]

# Make sure the target is unique
autosectionlabel_prefix_document = True

numfig = True
# numfig:
# numfig_number_figures = True
# numfig_figure_caption_prefix = "Figure"

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

templates_path = ['_templates']

# -- Options for HTML output

html_theme = 'sphinx_rtd_theme'

# -- Options for EPUB output
epub_show_urls = 'footnote'

numfig_format = {'figure': 'Figure %s', 'table': 'Table %s', 'code-block': 'Listing %s'}
