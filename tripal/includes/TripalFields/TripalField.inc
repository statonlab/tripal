<?php

class TripalField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default lable for this field.
  public static $default_label = 'Tripal Field';

  // The default description for this field.
  public static $default_description = 'The generic base class for all Tripal fields. ' .
    'Replace this text as appropriate for the child implementation.';

  // Provide a list of global settings. These can be accessed witihn the
  // globalSettingsForm.  When the globalSettingsForm is submitted then
  // Drupal will automatically change these settings for all fields.
  // Once instances exist for a field type then these settings cannot be
  // changed.
  public static $default_settings = array(
    'storage' => 'tripal_no_storage',
    // It is expected that all fields set a 'value' in the load() function.
    // In many cases, the value may be an associative array of key/value pairs.
    // In order for Tripal to provide context for all data, the keys should
    // be a controlled vocabulary term (e.g. rdfs:type). Keys in the load()
    // function that are supported by the query() function should be
    // listed here.
    'searchable_keys' => array(),
  );

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings  = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'schema',
    // The name of the term.
    'term_name' => 'Thing',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'Thing',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Inidates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should ahve auto_attach set to FALSE so tha their values can be
    // attached asyncronously.
  );

  // The default widget for this field.
  public static $default_widget = '';

  // The default formatter for this field.
  public static $default_formatter = '';

  // The module that manages this field.
  public static $module = 'tripal';

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = TRUE;

  // A boolean specifying that the field will not contain any data. This
  // should exclude the field from web serivces or downloads.  An example
  // could be a quick search field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = FALSE;


  // --------------------------------------------------------------------------
  //              PROTECTED CLASS MEMBERS -- DO NOT OVERRIDE
  // --------------------------------------------------------------------------
  // An array containing details about the field. The format of this array
  // is the same as that returned by field_info_fields()
  protected $field;

  // An array containing details about an instance of the field. A field does
  // not have to have an instance.  But if dealing with an instance (such as
  // when using the widgetForm, formatterSettingsForm, etc.) it should be set.
  protected $instance;


  // --------------------------------------------------------------------------
  //                     CONSTRUCTOR
  // --------------------------------------------------------------------------

  /**
   * Instantiates a new TripalField object.
   *
   * @param $field
   *   An array containing the field data as returned by field_info_field().
   * @param $instance
   *   An array containing the instance data as returned by field_instance_info().
   */
  public function __construct($field, $instance) {
    $this->field = $field;
    $this->instance = $instance;

    $class = get_called_class();

    if (!$instance) {
      tripal_set_message(t('Missing instance of field "%field"', array('%field' => $field['field_name'])), TRIPAL_ERROR);
    }
  }

  // --------------------------------------------------------------------------
  //           STATIC INFO FUNCTIONS -- DO NOT OVERRIDE
  // --------------------------------------------------------------------------

  /**
   * Provides default information about this field type
   *
   * This function corresponds to the hook_field_info() function of
   * the Drupal Field API.
   *
   * @return
   *   An array whose keys are field type names and whose values are arrays
   *   describing the field type. The keys are the same as for the
   *   hook_field_info() function.
   */
  public static function info() {
    $class = get_called_class();
    $info = array(
      'label' => $class::$default_label,
      'description' => $class::$default_description,
      'settings' => $class::$default_settings,
      'instance_settings' => $class::$default_instance_settings,
      'default_widget' => $class::$default_widget,
      'default_formatter' => $class::$default_formatter,
      'no_ui' => $class::$no_ui,
    );
    return $info;
  }


  // --------------------------------------------------------------------------
  //                 GETTERS AND SETTERS -- DO NOT OVERRIDE
  // --------------------------------------------------------------------------

  /**
   * Retrives the name of this field.
   *
   * @return
   *   This field's name.
   */
  public function getFieldName() {
    return $this->field['field_name'];
  }

  public function getField() {
    return $this->field;
  }
  public function getInstance() {
    return $this->instance;
  }

  /**
   * When constructing a pager for use by a field, all pagers must have
   * a unique ID
   */
  protected function getPagerElementID() {
    return $this->field['id'];
  }

  // --------------------------------------------------------------------------
  //                            OVERRIDEABLE FUNCTIONS
  // --------------------------------------------------------------------------

  /**
   *  Perform validation of the field regardless how it is updated.
   *
   *  Any errors encountered should be indicated by adding a value to the
   *  $errors array according to the instructions below.
   *
   *  @param $entity_type
   *    The type of $entity.
   *  @param $entity
   *    The entity for the operation.
   *  @param $field
   *    The field structure for the operation.
   *  @param $instance
   *    The instance structure for $field on $entity's bundle.
   *  @param $langcode
   *    The language associated with $items.
   *  @param $items
   *    $entity->{$field['field_name']}[$langcode], or an empty array if unset.
   *  @param $errors
   *    The array of errors (keyed by field name, language code, and delta) that
   *    have already been reported for the entity. The function should add its
   *    errors to this array. Each error is an associative array with the
   *    following keys and values:
   *      - error: An error code (should be a string prefixed with the
   *        module name).
   *      - message: The human readable message to be displayed.
   *
   */
  public function validate($entity_type, $entity, $field, $items, &$errors) {

  }


  /**
   * Loads the field values from the underlying data store.
   *
   * @param $entity
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'] = $value;
   *   where:
   *     - $entity is the enity object to which this field is attached.
   *     - $field_name is the name of this field
   *     - 'und' is the language code (in this case 'und' == undefined)
   *     - 0 is the cardinality.  Increment by 1 when more than one item is
   *       available.
   *     - 'value' is the key indicating the value of this field. It should
   *       always be set.  The value of the 'value' key will be the contents
   *       used for web services and for downloadable content.  The value
   *       should be of the follow format types: 1) A single value (text,
   *       numeric, etc.) 2) An array of key value pair. 3) If multiple entries
   *       then cardinality should incremented and format types 1 and 2 should
   *       be used for each item.
   *   The array may contain as many other keys at the same level as 'value'
   *   but those keys are for internal field use and are not considered the
   *   value of the field.
   *
   *
   */
  public function load($entity) {

  }

  /**
   * Provides a form for the 'Field Settings' of the field management page.
   *
   * This is an optional hook function and is similar to the
   * hook_field_settings_form function().
   *
   * @param $has_data
   *   TRUE if the field already has data, FALSE if not.
   */
  public function settingsForm($has_data) {
    $settings = $this->field['settings'];
    $element = array();

    $element['#field'] = $this->field;
    $element['#instance'] = $this->instance;
    $element['#element_validate'][] = 'tripal_field_settings_form_validate';

    return $element;
  }
  /**
   *
   * @param $form
   * @param $form_state
   */
  public function settingsFormValidate($form, &$form_state) {

  }

  /**
   * Describes this field to Views.
   *
   * An array of views data, in the same format as the return value of
   * hook_views_data().
   *
   */
  public function viewsData() {
    $data = array();

    $bundle_name = $this->instance['bundle'];
    $field_name = $this->field['field_name'];

    // Fields should be associated with the bundle's term identifier
    // (i.e. [vocab]__[accession].
    $bundle = tripal_load_bundle_entity(array('name' => $bundle_name));
    $term = tripal_load_term_entity(array('term_id' => $bundle->term_id));
    $table = $term->vocab->vocabulary . '__' . $term->accession;

    $data[$table][$field_name] = array(
      'title' => $this->instance['label'],
      'help' => $this->instance['description'],
      'field' => array(
        'handler' => 'tripal_views_handler_field',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'tripal_views_handler_filter_string',
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
    );

    return $data;
  }

  /**
   * Provides a form for the 'Field Settings' of an instance of this field.
   *
   * This function corresponds to the hook_field_instance_settings_form()
   * function of the Drupal Field API.
   *
   * Validation of the instance settings form is not supported by Drupal, but
   * the TripalField class does provide a mechanism for supporting validation.
   * To allow for validation of your setting form you must call the parent
   * in your child class:
   *
   * @code
   *   $element = parent::instanceSettingsForm();
   * @endcode
   *
   * Please note, the form generated with this function does not easily
   * support AJAX calls in the same way that other Drupal forms do.  If you
   * need to use AJAX you must manually alter the $form in your ajax call.
   * The typical way to handle updating the form via an AJAX call is to make
   * the changes in the form function itself but that doesn't work here.
   */
  public function instanceSettingsForm() {
    $settings = $this->instance['settings'];
    $element = array();

    $element['#field'] = $this->field;
    $element['#instance'] = $this->instance;
    $element['#element_validate'][] = 'tripal_field_instance_settings_form_validate';
    return $element;
  }
  /**
  * Provides validation of the instance settings form.
  *
  * There is no equivalent function in the Drupal Field API. Validation
  * of instance settings forms in Drupal is not supported. However, the
  * TripalField provides this function to fill the gap.  See the
  * documentation for the instanceSettingsForm() function for instructions
  * to support use of this function.
  *
  * @param $form
  * @param $form_state
  */
  public function instanceSettingsFormValidate($form, &$form_state) {

  }

  /**
   * Afte a field instance is created the following function is run.
   *
   * This function is equivalent to the hook_field_create_field() hook of
   * the Drupal Field API. This function is invoked after a new field
   * instance is created.
   */
  public function createInstance() {

  }

  /**
   * Used to filter records that match a given condition.
   *
   * Entities can be filtered using the fields.  This function should be
   * implemented if the field  supports filtering.  To provide filtering,
   * the $query object should be updated to including any joins and
   * conditions necessary. The following rules should be followed when
   * implementing this function:
   * - Any keys from the value array that support filtering should be set
   *   in the $default_settings['searchable_keys'] array of this class file.
   * - Implement support for every key in the
   *   $default_settings['searchable_keys'] array.
   * - However, avoid making filteres for non-indexed database columns.
   * - This function should never set the fields that should be returned
   *   nor ordering or group by.
   *
   * @param $query
   *   A query object appropriate for the data storage backend. For example,
   *   The Tripal Chado module will provide a SelectQuery object.
   * @param $condition
   *   The field specific condition as set in the TripalFieldQuery object.
   */
  public function query($query, $condition) {

  }

  /**
   * Used to sort records that have been filtered.
   *
   * @param $query
   *   A query object appropriate for the data storage backend. For example,
   *   The Tripal Chado module will provide a SelectQuery object.
   * @param $order
   *   The field ordering as set in the TripalFieldQuery object.  This function
   *   should handle the ordering request as specified by this object.
   */
  public function queryOrder($query, $order) {

  }
}