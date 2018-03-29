<?php

/**
 * @class
 * Purpose:
 *
 * Data:
 * Assumptions:
 */
class data__ontology_data extends TripalField {

  //static constants
  public static $default_label = 'Ontology data';
  public static $default_description = 'Ontology mappings of related entities.';
  public static $default_widget = 'data__ontology_data_widget';
  public static $default_formatter = 'data__ontology_data_formatter';
  public static $module = 'tripal';

  // A list of global settings. These can be accessed within the
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
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = array(
    // The DATABASE name, as it appears in chado.db.  This also builds the link-out url.  In most cases this will simply be the CV name.  In some cases (EDAM) this will be the SUBONTOLOGY.
    'term_vocabulary' => 'data',
    // The name of the term.
    'term_name' => 'Ontology data',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '4231',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Indicates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should have auto_attach set to FALSE so tha their values can be
    // attached asynchronously.
    'auto_attach' => FALSE,
  );

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;

  // A boolean specifying that the field will not contain any data. This
  // should exclude the field from web services or downloads.  An example
  // could be a quick search field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = FALSE;

  /**
   * Loads the field values from the underlying data store.
   *
   * @param $entity
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'] = $value;
   *   where:
   *     - $entity is the entity object to which this field is attached.
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
   */
  public function load($entity) {
    parent::load($entity);
    $entity_id = $entity->id;
    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];

    $entity->{$field_name}['und'][0]['value'] = NULL;


    $child_entities = $this - fetch_associated_features($entity);
    //this is an array with key 1 = bundle, key 2 = entity ids. (in case we need to filter by bundle type).

    //retrieve cvterms from cvterm->entity index
    //TODO: make it so this is set in the field settings
    $cv_id = 10;

    //TODO:  look up which target bundle to include based on field settings instead
    $target_bundles = [10];

    $child_entity_cvterms = [];


    $query = db_select("chado.cv", 'CV')
      ->join("chado.cvterm", "CVT", "CV.cv_id = CVT.cv_id")
      ->fields("CVT", "cvterm_id")
      ->join("public.cvterm_entity_index", "CEI", "CVT.cvterm_id = CEI.cvterm_id")
      ->fields("CEI", ["bundle_id", "entities"])
      ->condition("bundle_id", $target_bundles, 'IN');


    $results = $query->execute();
    //NEXT: filter the array to just entities mapped on this instance of the field
    foreach ($results as $result) {
      $cvterm_id = $result->cvterm_id;
      $entities_serial = $result->entities;
      $entities = unserialize($entities_serial);

      foreach ($target_bundles as $bundle_id) {
        $mapped_entities = $child_entities[$bundle_id];
        $matched = array_intersect_key($mapped_entities, $entities);
        //TODO:  what do we want the end object to be?
        //An array with CVTERM[bundle][entity_id] = ['title', 'relationshiptype', 'pathlength'] which is a combination of the two arrays right?
        $child_entity_cvterms[$cvterm_id] = $matched;
      }
    }

    $entity->{$field_name}['und'][0]['value'] = $child_entity_cvterms;


    //maybe hte formatter does this instead...
    drupal_add_js(array(
      'tripal' => array(
        'cv_lookup_entity_targets' => $child_entity_cvterms
      ),
    ), 'setting');

  }


  /**
   * This function should probably be moved out of the field and performed at
   * regular intervals and teh results stored in a table. For now, it runs on
   * load, and retrieves ALL ENTITIES associated with the anchor entity.
   *
   * @param $entity
   *
   * @return array
   */
  private function fetch_associated_features($entity) {
    $entity_id = $entity->id;
    $bundle = tripal_load_bundle_entity(array('name' => $entity->bundle));
    $record_id = $entity->record_id;
    $child_entities = [];

    //If organism, we'll load all the features associated with it in the type_id column
    if ($bundle->data_table == 'organism') {
      $query = db_select('public.tripal_bundle', 'tb')
        ->fields('bundle_id', 'type_id')
        ->condition('data_table', '=', 'feature');
      $feature_bundles = $query->fetchAll();

      foreach ($feature_bundles as $fb) {
        $type_id = $fb->type_id;
        $bundle_id = $fb->bundle_id;
        $bundle_table = "public.chado_bio_data_" . $bundle_id;

        //fetch all features of that type associated with this organism
        //TODO: ALSO FETCH TITLES OF CHILDREN
        $query = db_select($bundle_table, 'bt');
        $query->fields('bt', ['record_id', 'entity_id'])
          ->join('chado.feature', "cf", 'bt.record_id = cf.feature_id')
          ->condition('cf.organism_id', '=', $record_id);
        $results = $query->fetchAll();

        if ($results) {
          $child_entities[$bundle_table] = $results;
        }
      }
    }

    return $child_entities;
  }


}
