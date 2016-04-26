<?php
/**
 * Implements hook_info() for fields.
 *
 * This is a hook provided by the tripal_chado module for offloading the
 * hook_field_info() hook for each field to specify.
 */
function chado_linker__pub_info() {
  return array(
    'label' => t('Publications'),
    'description' => t('Associates a publication (e.g. journal article,
          conference proceedings, book chapter, etc.) with this record.'),
    'default_widget' => 'chado_linker__pub_widget',
    'default_formatter' => 'chado_linker__pub_formatter',
    'settings' => array(),
    'storage' => array(
      'type' => 'field_chado_storage',
      'module' => 'tripal_chado',
      'active' => TRUE
    ),
  );
}
/**
 * Implements hook_attach_info().
 *
 * This is a hook provided by the tripal_Chado module. It allows the field
 * to specify which bundles it will attach to and to specify thee settings.
 *
 * @param $entity_type
 * @param $entity
 * @param $term
 *
 * @return
 *   A field array
 */
function chado_linker__pub_attach_info($entity_type, $bundle, $target) {
  $field_info = array();

  $table_name = $target['data_table'];
  $type_table = $target['type_table'];
  $type_field = $target['field'];
  $cv_id      = $target['cv_id'];
  $cvterm_id  = $target['cvterm_id'];

  // If the linker table does not exists then we don't want to add attach.
  $pub_table = $table_name . '_pub';
  if (!chado_table_exists($pub_table)) {
    return $field_info;
  }

  $schema = chado_get_schema($pub_table);
  $pkey = $schema['primary key'][0];

  // Initialize the field array.
  $field_info = array(
    'field_name' => $table_name . '__pub',
    'field_type' => 'chado_linker__pub',
    'widget_type' => 'chado_linker__pub_widget',
    'widget_settings' => array('display_label' => 1),
    'description' => '',
    'label' => 'Publications',
    'is_required' => 0,
    'cardinality' => FIELD_CARDINALITY_UNLIMITED,
    'storage' => 'field_chado_storage',
    'field_settings' => array(
      'chado_table' => $pub_table,
      'chado_column' => $pkey,
      'base_table' => $table_name,
      'semantic_web' => array(
        'type' => '',
        'ns' => '',
        'nsurl' => '',
      ),
    ),
  );
  return $field_info;
}
/**
 * Implements hook_widget_info.
 *
 * This is a hook provided by the tripal_chado module for offloading
 * the hook_field_widget_info() hook for each field to specify.
 */
function chado_linker__pub_widget_info() {
  return array(
    'label' => t('Publications'),
    'field types' => array('chado_linker__pub'),
  );
}
/**
 * Implements hook_formatter_info.
 *
 * This is a hook provided by the tripal_chado module for
 * offloading the hook_field_formatter_info() for each field
 * to specify.
 *
 */
function chado_linker__pub_formatter_info() {
  return array(
    'label' => t('Publications'),
    'field types' => array('chado_linker__pub'),
    'settings' => array(
    ),
  );
}
/**
 *
 * @param unknown $entity_type
 * @param unknown $entity
 * @param unknown $field
 * @param unknown $instance
 * @param unknown $langcode
 * @param unknown $items
 * @param unknown $display
 */
function chado_linker__pub_formatter(&$element, $entity_type, $entity, $field,
    $instance, $langcode, $items, $display) {

  $list_items = array();
  $chado_table = $field['settings']['chado_table'];
  foreach ($items as $delta => $item) {
    if ($item[$chado_table . '__pub_id']) {
      $pub = chado_generate_var('pub', array('pub_id' => $item[$chado_table . '__pub_id']));
      $list_items[$pub->pyear] = $pub->uniquename;
    }
  }

  krsort($list_items, SORT_NUMERIC);

  $list = array(
    'title' => '',
    'items' => $list_items,
    'type' => 'ol',
    'attributes' => array(),
  );
  $element[0] = array(
    '#type' => 'markup',
    '#markup' => theme_item_list($list),
  );
}
/**
 *
 * @param unknown $field_name
 * @param unknown $widget
 * @param unknown $form
 * @param unknown $form_state
 * @param unknown $field
 * @param unknown $instance
 * @param unknown $langcode
 * @param unknown $items
 * @param unknown $delta
 * @param unknown $element
 */
function chado_linker__pub_widget(&$widget, $form, $form_state, $field,
    $instance, $langcode, $items, $delta, $element) {

  $entity = $form['#entity'];
  $field_name = $field['field_name'];

  // Get the FK column that links to the base table.
  $table_name = $field['settings']['chado_table'];
  $base_table = $field['settings']['base_table'];
  $schema = chado_get_schema($table_name);
  $pkey = $schema['primary key'][0];
  $fkeys = array_values($schema['foreign keys'][$base_table]['columns']);
  $fkey = $fkeys[0];

  // Get the field defaults.
  $record_id = '';
  $fkey_value = $element['#entity']->chado_record_id;
  $pub_id = '';
  $title = '';

  // If the field already has a value then it will come through the $items
  // array.  This happens when editing an existing record.
  if (array_key_exists($delta, $items)) {
    $record_id = $items[$delta]['value'];
    $fkey_value = $items[$delta][$table_name . '__' . $fkey];
    $pub_id = $items[$delta][$table_name . '__pub_id'];
    $title = $items[$delta][$table_name . '--pub__uniquename'];
  }

  // Check $form_state['values'] to see if an AJAX call set the values.
  if (array_key_exists('values', $form_state) and array_key_exists($delta, $form_state['values'])) {
    $record_id = tripal_chado_get_field_form_values($table_name, $form_state, $delta, $table_name);
    $fkey_value = tripal_chado_get_field_form_values($table_name, $form_state, $delta, $table_name . '__' . $fkey);
    $pub_id = tripal_chado_get_field_form_values($table_name, $form_state, $delta, $table_name . '__pub_id');
    $title = tripal_chado_get_field_form_values($table_name, $form_state, $delta, $table_name . '__title');
  }

  $schema = chado_get_schema('pub');

  $widget['#table_name'] = $table_name;
  $widget['#fkey_field'] = $fkey;
  $widget['#element_validate'] = array('chado_linker__pub_widget_validate');
  $widget['#theme'] = 'chado_linker__pub_widget';
  $widget['#prefix'] =  "<span id='$table_name-$delta'>";
  $widget['#suffix'] =  "</span>";

  $widget['value'] = array(
    '#type' => 'value',
    '#default_value' => $record_id,
  );
  $widget[$table_name . '__' . $fkey] = array(
    '#type' => 'value',
    '#default_value' => $fkey_value,
  );
  $widget[$table_name . '__pub_id'] = array(
    '#type' => 'value',
    '#default_value' => $pub_id,
  );

  $widget[$table_name . '--pub__uniquename'] = array(
    '#type' => 'textfield',
    '#title' => t('Publication ID'),
    '#default_value' => $title,
    '#autocomplete_path' => 'admin/tripal/storage/chado/auto_name/pub',
    '#ajax' => array(
      'callback' => "chado_linker__pub_widget_form_ajax_callback",
      'wrapper' => "$table_name-$delta",
      'effect' => 'fade',
      'method' => 'replace'
    ),
    '#maxlength' => 100000,
  );
}
/**
 * An Ajax callback for the pub widget.
 */
function chado_linker__pub_widget_form_ajax_callback($form, $form_state) {

  $field_name = $form_state['triggering_element']['#parents'][0];
  $delta = $form_state['triggering_element']['#parents'][2];

  return $form[$field_name]['und'][$delta];
}

/**
 * Callback function for validating the chado_linker__pub_widget.
 */
function chado_linker__pub_widget_validate($element, &$form_state) {

  $field_name = $element['#field_name'];
  $delta = $element['#delta'];
  $table_name = $element['#table_name'];
  $fkey = $element['#fkey_field'];

  // If the form ID is field_ui_field_edit_form, then the user is editing the
  // field's values in the manage fields form of Drupal.  We don't want
  // to validate it as if it were being used in a data entry form.
  if ($form_state['build_info']['form_id'] =='field_ui_field_edit_form') {
    return;
  }

  // Get the field values.
  $fkey_value = tripal_chado_get_field_form_values($table_name, $form_state, $delta, $table_name . '__' . $fkey);
  $pub_id = tripal_chado_get_field_form_values($table_name, $form_state, $delta, $table_name . '__pub_id');
  $uname = tripal_chado_get_field_form_values($table_name, $form_state, $delta, $table_name . '--pub__uniquename');

  // If the user provided a uniquename then we want to set the
  // foreign key value to be the chado_record_idd
  if ($uname) {

    // Get the pub. If one with the same name and type is already present
    // then use that. Otherwise, insert a new one.
    if (!$pub_id) {
      $pub = chado_generate_var('pub', array('uniquename' => $uname));
      // Set the pub_id and FK value
      tripal_chado_set_field_form_values($field_name, $form_state, $pub->pub_id, $delta, $table_name . '__pub_id');
      $fkey_value = $element['#entity']->chado_record_id;
      tripal_chado_set_field_form_values($field_name, $form_state, $fkey_value, $delta, $table_name . '__' . $fkey);
    }

  }
  else {
    // If the $syn_name is not set, then remove the linker FK value to the base table.
    tripal_chado_set_field_form_values($field_name, $form_state, '', $delta, $table_name . '__' . $fkey);
    tripal_chado_set_field_form_values($field_name, $form_state, '', $delta, $table_name . '__pub_id');
  }

}
/**
 * Theme function for the pub widget.
 *
 * @param $variables
 */
function theme_chado_linker__pub_widget($variables) {
  $element = $variables['element'];

  // These two fields were added to the widget to help identify the fields
  // for layout.
  $table_name = $element['#table_name'];
  $fkey = $element['#fkey_field'];

  $layout = "
    <div class=\"pub-widget\">
      <div class=\"pub-widget-item\">" .
        drupal_render($element[$table_name . '--pub__uniquename']) . "
      </div>
    </div>
  ";

  return $layout;
}

/**
 * Loads the field values with appropriate data.
 *
 * This function is called by the tripal_chado_field_storage_load() for
 * each property managed by the field_chado_storage storage type.  This is
 * an optional hook function that is only needed if the field has
 * multiple form elements.
 *
 * @param $field
 * @param $entity
 * @param $base_table
 * @param $record
 */
function chado_linker__pub_load($field, $entity, $base_table, $record) {

  $field_name = $field['field_name'];
  $field_type = $field['type'];
  $field_table = $field['settings']['chado_table'];
  $field_column = $field['settings']['chado_column'];

  // Get the FK that links to the base record.
  $schema = chado_get_schema($field_table);
  $pkey = $schema['primary key'][0];
  $fkey_lcolumn = key($schema['foreign keys'][$base_table]['columns']);
  $fkey_rcolumn = $schema['foreign keys'][$base_table]['columns'][$fkey_lcolumn];

  // Set some defaults for the empty record.
  $entity->{$field_name}['und'][0] = array(
    'value' => '',
    $field_table . '__' . $fkey_lcolumn => '',
    $field_table . '__' . 'pub_id' => '',
    $field_table . '--' . 'pub__uniquename' => '',
  );

  $linker_table = $base_table . '_pub';
  $options = array(
    'return_array' => 1,
  );
  $record = chado_expand_var($record, 'table', $linker_table, $options);

  if (count($record->$linker_table) > 0) {
    $i = 0;
    foreach ($record->$linker_table as $index => $linker) {
      $pub = $linker->pub_id;
      $entity->{$field_name}['und'][$i] = array(
        'value' => $linker->$pkey,
        $field_table . '__' . $fkey_lcolumn => $linker->$fkey_lcolumn->$fkey_lcolumn,
        $field_table . '__' . 'pub_id' => $pub->pub_id,
        $field_table . '--' . 'pub__uniquename' => $pub->uniquename,
      );
      $i++;
    }
  }
}
