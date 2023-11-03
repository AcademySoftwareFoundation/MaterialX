{{ fullname | escape | underline}}

.. automodule:: {{ fullname }}

   Alphabetical Index
   ------------------

   {% block attributes %}
   {% if attributes %}
   .. rubric:: {{ _('Module Attributes') }}

   .. autosummary::
   {% for item in attributes %}
      {{ item }}
   {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block functions %}
   {% if functions %}
   .. rubric:: {{ _('Functions') }}

   .. autosummary::
   {% for item in functions %}
      {{ item }}
   {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block classes %}
   {% if classes %}
   .. rubric:: {{ _('Classes') }}

   .. autosummary::
   {% for item in classes %}
      {{ item }}
   {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block exceptions %}
   {% if exceptions %}
   .. rubric:: {{ _('Exceptions') }}

   .. autosummary::
   {% for item in exceptions %}
      {{ item }}
   {%- endfor %}
   {% endif %}
   {% endblock %}

{% block modules %}
{% if modules %}
.. rubric:: Modules

.. autosummary::
   :toctree:
   :recursive:
{% for item in modules %}
   {{ item }}
{%- endfor %}
{% endif %}
{% endblock %}

Module source:
`MaterialX <https://github.com/AcademySoftwareFoundation/MaterialX/tree/main>`_ /
`source <https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/source>`_ /
`PyMaterialX <https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/source/PyMaterialX>`_ /
`{{ fullname }} <https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/source/PyMaterialX/{{ fullname }}>`_ /
