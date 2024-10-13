{{ fullname | escape | underline}}

.. currentmodule:: {{ module }}

.. autoclass:: {{ objname }}
   :show-inheritance:

   {% block methods %}
   {% if methods %}
   .. rubric:: {{ _('Methods') }}

   .. autosummary::
   {% for item in methods %}
      {# Only consider methods that are local to the current object #}
      {%- if has_member(module, objname, item) %}
      ~{{ name }}.{{ item }}
      {%- endif %}
   {%- endfor %}
   {%- endif %}
   {%- endblock %}

   {%- block attributes %}
   {%- if attributes %}
      {# Split the attributes into those defining names of attributes and other properties #}
      {%- set ns = namespace(attribute_names_present=false, properties_present=false) %}
      {%- for item in attributes %}
         {# Only consider attributes that are local to the current object #}
         {%- if has_member(module, objname, item) %}
            {# Consider the current item an attribute if it contains "_ATTRIBUTE" in #}
            {# its name, e.g. "INTERFACE_NAME_ATTRIBUTE" #}
            {%- if "_ATTRIBUTE" in item %}
               {%- set ns.attribute_names_present = true %}
            {%- else %}
               {%- set ns.properties_present = true %}
            {%- endif %}
         {%- endif %}
      {%- endfor %}

      {% if ns.attribute_names_present %}
   Attribute Names
   ---------------
         {%- for item in attributes %}
            {%- if "_ATTRIBUTE" in item and has_member(module, objname, item) %}
   .. autoattribute:: {{ module }}.{{ objname }}.{{ item }}
            {%- endif %}

         {%- endfor %}
      {%- endif %}

      {% if ns.properties_present %}
   Properties
   ----------

         {%- for item in attributes %}
            {%- if has_member(module, objname, item) %}
               {%- if not "_ATTRIBUTE" in item %}
   .. autoattribute:: {{ module }}.{{ objname }}.{{ item }}
               {%- endif %}
            {%- endif %}

         {%- endfor %}
      {%- endif %}

   {% endif %}
   {% endblock %}

   {% block automethods %}
   {% if methods %}

   Methods
   -------

   {% for item in methods %}
      {%- if has_member(module, objname, item) %}
   .. automethod:: {{ module }}.{{ objname }}.{{ item }}
      {%- endif %}
   {%- endfor %}
   {% endif %}
   {% endblock %}
