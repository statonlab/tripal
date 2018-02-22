<?php

class chado_linker__prop_widget extends ChadoFieldWidget {
  // The default lable for this field.
  public static $default_label = 'Chado Property';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('chado_linker__prop');

  /**
   *
   * @see TripalFieldWidget::form()
   */
  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {
    parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $base_table = $this->instance['settings']['base_table'];
    $field_table = $this->instance['settings']['chado_table'];
    $chado_column = $this->instance['settings']['chado_column'];
    $rows = array_key_exists('rows', $this->instance['settings']) ? $this->instance['settings']['rows'] : 1;
    $instance = $this->instance;

    // Get the name of the pkey field for this property table and the name
    // of the FK field that links to the base table.
    $schema = chado_get_schema($field_table);
    $pkey = $schema['primary key'][0];
    $lfkey_field = key($schema['foreign keys'][$base_table]['columns']);
    $rfkey_field = $schema['foreign keys'][$base_table]['columns'][$lfkey_field];

    // Get the field defaults.
    $record_id = '';
    $fk_value = '';
    $value = '';
    $rank = $delta;
    $type_id = '';
    $cvalue_id = '';

    // If the field already has a value then it will come through the $items
    // array.  This happens when editing an existing record.
    if (count($items) > 0) {
      // Check for element values that correspond to fields in the Chado table.
      $fk_value = tripal_get_field_item_keyval($items, 0, 'chado-' . $field_table . '__' . $lfkey_field, $fk_value);
      $type_id = tripal_get_field_item_keyval($items, 0, 'chado-' . $field_table . '__type_id', $type_id);
      if (array_key_exists($delta, $items)) {
        $record_id = tripal_get_field_item_keyval($items, $delta, 'chado-' . $field_table . '__' . $pkey, $record_id);
        $value = tripal_get_field_item_keyval($items, $delta, 'chado-' . $field_table . '__value', $value);
        $rank = tripal_get_field_item_keyval($items, $delta, 'chado-' . $field_table . '__rank', $rank);
        $cvalue_id = tripal_get_field_item_keyval($items, $delta, 'chado-' . $field_table . '__cvalue_id', $cvalue_id);

      }
    }

    // Check $form_state['values'] to see if an AJAX call set the values.
    if (array_key_exists('values', $form_state) and
      array_key_exists($field_name, $form_state['values'])) {
      $record_id = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__' . $pkey];
      $fk_value = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__' . $lfkey_field];
      $type_id = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__type_id'];
      $value = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__value'];
      $rank = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__rank'];
      $cvalue_id = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__cvalue_id'];

    }

    // Use default value for the field if it's not already set
    if (!$value && isset($instance['default_value'][$delta])) {
      $value = $instance['default_value'][$delta]['value'];
    }
    if (!$type_id) {
      $vocabulary = $this->instance['settings']['term_vocabulary'];
      $accession = $this->instance['settings']['term_accession'];
      $cvterm = tripal_get_cvterm([
        'dbxref_id' => [
          'db_id' => [
            'name' => $vocabulary,
          ],
          'accession' => $accession,
        ],
      ]);
      if ($cvterm) {
        $type_id = $cvterm->cvterm_id;
      }
    }

    $widget['value'] = [
      '#type' => 'value',
      '#value' => array_key_exists($delta, $items) ? $items[$delta]['value'] : '',
    ];

    $widget['chado-' . $field_table . '__' . $pkey] = [
      '#type' => 'hidden',
      '#default_value' => $record_id,
    ];
    $widget['chado-' . $field_table . '__' . $lfkey_field] = [
      '#type' => 'hidden',
      '#value' => $fk_value,
    ];
    $widget['chado-' . $field_table . '__value'] = [
      '#type' => 'textarea',
      '#default_value' => $value,
      '#title' => $instance['label'],
      '#description' => $instance['description'],
      '#rows' => $rows,
      '#required' => $instance['required'],
    ];
    $widget['chado-' . $field_table . '__type_id'] = [
      '#type' => 'hidden',
      '#value' => $type_id,
    ];
    $widget['chado-' . $field_table . '__rank'] = [
      '#type' => 'hidden',
      '#value' => $rank,
    ];

    $widget['chado-' . $field_table . '__cvalue_id'] = [
      '#type' => 'hidden',
      '#value' => $cvalue_id
    ];

    $widget['cvalue_term'] = array(
      '#type' => 'fieldset',
      '#title' =>  'Value Controlled Vocabulary Term',
      '#prefix' => '<div id = "chado-' . $field_table . $type_id . '__cvalue_term">',
      '#suffix' => '</div>',
    );

    $widget['cvalue_term']['cvalue_text'] = [
      '#type' => 'textfield',
      '#title' => 'Set property value using a cv term.',
      '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/cvterm/",
    ];

    $widget['cvalue_term']['select_button'] = [
      '#type' => 'button',
      '#value' => t('Lookup Term'),
      '#name' => 'select_cvterm',
      '#ajax' => [
        'callback' => "cvalue_ajax_callback",
        'wrapper' => "chado-' . $field_table .'_' . $type_id . '__cvalue_term",
        'effect' => 'fade',
        'method' => 'replace'
      ],
    ];

    // If a new term name has been specified by the user then give some extra
    // fields to clarify the term.
    $term_name = '';
    if (array_key_exists('values', $form_state) and array_key_exists('new_name', $form_state['values'])) {
      $term_name = array_key_exists('values', $form_state) ? $form_state['values']['cvalue_text'] : '';
    }
    if (array_key_exists('input', $form_state) and array_key_exists('new_name', $form_state['input'])) {
      $term_name = array_key_exists('input', $form_state) ? $form_state['input']['cvalue_text'] : '';
    }
    if ($term_name) {
      $widget['cvalue_term']['instructions'] = array(
        '#type' => 'item',
        '#title' => 'Matching terms',
        '#markup' => t('Please select the term the best matches the
        content type you want to associate with this field. If the same term exists in
        multiple vocabularies you will see more than one option below.')
      );
      $match = array(
        'name' => $term_name,
      );
      $terms = chado_generate_var('cvterm', $match, array('return_array' => TRUE));
      $terms = chado_expand_var($terms, 'field', 'cvterm.definition');
      $num_terms = 0;
      foreach ($terms as $term) {
        // Save the user a click, by setting the default value as 1 if there's
        // only one matching term.
        $default = FALSE;
        $attrs = array();
        if ($num_terms == 0 and count($terms) == 1) {
          $default = TRUE;
          $attrs = array('checked' => 'checked');
        }
        $widget['cvalue_term']['term-' . $term->cvterm_id] = array(
          '#type' => 'checkbox',
          '#title' =>  $term->name,
          '#default_value' => $default,
          '#attributes' => $attrs,
          '#description' => '<b>Vocabulary:</b> ' . $term->cv_id->name . ' (' . $term->dbxref_id->db_id->name . ') ' . $term->cv_id->definition .
            '<br><b>Term: </b> ' . $term->dbxref_id->db_id->name . ':' . $term->dbxref_id->accession . '.  ' .
            '<br><b>Definition:</b>  ' . $term->definition,
        );
        $num_terms++;
      }
      if ($num_terms == 0) {
        $widget['cvalue_term']['none'] = array(
          '#type' => 'item',
          '#markup' => '<i>' . t('There is no term that matches the entered text.') . '</i>'
        );
      }
    }



  }


  /**
   * Implements an AJAX callback for the tripal_field_vocab_select_term_form.
   */
  public function cvalue_ajax_callback($form, &$form_state) {
    return $form;
  }


  /**
   *
   * @see TripalFieldWidget::submit()
   */
  public function validate($element, $form, &$form_state, $langcode, $delta) {
    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $base_table = $this->instance['settings']['base_table'];
    $field_table = $this->instance['settings']['chado_table'];
    $chado_column = $this->instance['settings']['chado_column'];
    $instance = $this->instance;

    $schema = chado_get_schema($field_table);
    $pkey = $schema['primary key'][0];
    $base_table = $this->instance['settings']['base_table'];
    $lfkey_field = key($schema['foreign keys'][$base_table]['columns']);
    $rfkey_field = $schema['foreign keys'][$base_table]['columns'][$lfkey_field];

    $value = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__value'];
    $cvalue_id = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__cvalue_id'];

    if (!$value && $cvalue_id){
    //User has entered a cvalue, so use that term for the value as well for backwards compatibility.

      $cvterm = tripal_get_cvterm(array('cvterm_id' => $cvalue_id));
      $value = $cvterm->name;

    }
    $form_state['values'][$field_name]['und'][$delta]['value'] = $value;

    // If the user removed the property then we want to clear out the other
    // fields except the pkey value. If the pkey field is present and the value
    // is present then the chado storage backend will delete the record.
    if (!$value) {
      $form_state['values'][$field_name]['und'][$delta]['value'] = 'delete_me';
      $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__' . $lfkey_field] = '';
      $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__type_id'] = '';
      $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__value'] = '';
      $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__rank'] = '';
    }
  }
}
