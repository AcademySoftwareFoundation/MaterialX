{{ fullname | escape | underline}}

.. automodule:: {{ fullname }}

{% block classes %}
{% if classes %}
Classes
-------

.. autosummary::
   :toctree:

{% for item in classes %}
   {{ item }}
{%- endfor %}
{% endif %}
{% endblock %}

{% block exceptions %}
{% if exceptions %}
Exception Types
---------------

.. autosummary::
   :toctree:

{% for item in exceptions %}
   {{ item }}
{%- endfor %}
{% endif %}
{% endblock %}

{% block functions %}
{% if functions %}
Module Functions
----------------

.. autosummary::
{% for item in functions %}
   {{ item }}
{%- endfor %}

{{ (fullname + " Module Function Documentation") | underline("^") }}
{% for item in functions %}
.. autofunction:: {{ item }}
{%- endfor %}
{% endif %}
{% endblock %}

Module source:
`MaterialX <https://github.com/AcademySoftwareFoundation/MaterialX/tree/main>`_ /
`source <https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/source>`_ /
`PyMaterialX <https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/source/PyMaterialX>`_ /
`{{ fullname }} <https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/source/PyMaterialX/{{ fullname }}>`_ /
