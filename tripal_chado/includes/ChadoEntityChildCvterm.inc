<?php

class ChadoEntityChildCvterm {


  public function update_all_entities() {
    //First, empty db
    db_truncate("public.tripal_cvterm_entity_index")->execute();

    $o_array = [];

    $bundles = $this->get_all_bundles();

    foreach ($bundles as $bundle) {

      $cvterm_tracker = [];
      if ($bundle->data_table !== "feature" && $bundle->data_table !== "biomaterial") {
        continue;
      }

      $cbd_table = "public.chado_" . $bundle->name;
      //get all entities in this bundle
      $entities = db_select($cbd_table, 'cbd')->fields('cbd', [
        'entity_id',
        'record_id',
      ])->execute()->fetchAll();

      foreach ($entities as $entity) {
        $types = NULL;
        $eid = $entity->entity_id;
        $rid = $entity->record_id;
        //directly associated cvterms
        $cvterms = $this->get_all_associated_cvterms($bundle->data_table, $rid);
        //why is this always NULL?

        if (!$cvterms) {
          continue;
        }

        foreach ($cvterms as $cvterm => $relationship) {
          $cvterm_tracker[$cvterm][$eid]["relationship"] = $relationship;
          $cvterm_tracker[$cvterm][$eid]["path_length"] = 0;
        }
      }
      //Loop through the cvterms, retrieve all child terms defined in cvtermpath, and add them in copying the relationship type
      foreach ($cvterm_tracker as $cvterm => $entity_list) {
        $children = $this->fetch_ancestors($cvterm);

      //TODO: This code is current untested, need terms with ancestors
        foreach ($children as $child_cvterm => $ancestor_info) {
          //add each individual ancestor term
          $entities = array_keys($entity_list);
          foreach ($entities as $entity) {
            $cvterm_tracker[$child_cvterm][$entity]["path_length"] = $ancestor_info["path_length"];
            $cvterm_tracker[$child_cvterm][$entity]["relationship"] = $cvterm_tracker[$cvterm][$entity]["relationship"];
          }
        }
      }

      //Insert into public.tripal_cvterm_entity_index

      foreach ($cvterm_tracker as $cvterm => $entities) {
        $serialized_entities = serialize($entities);
        db_insert("public.tripal_cvterm_entity_index")
          ->fields([
            "cvterm_id" => $cvterm,
            "bundle_id" => $bundle->id,
            "entities_array" => $serialized_entities,
          ])
          ->execute();
      }
    }
  }


  public function fetch_ancestors($cvterm) {

    if (!$cvterm) {
      print ("Error: No CVterm provided");

      return NULL;
    }

    $ancestors = [];

    $query = db_select("chado.cvtermpath", 't')
      ->fields('t', ['object_id', 'pathdistance']);
    $query->condition('subject_id', $cvterm);
    //question: should we do an inner join here to ensure that the CV ID is the same in both subject and object?
    $query->execute();


    foreach ($query as $result) {
      $new_term = $result->object_id;
      $path = $result->pathdistance;
      $ancestors[$new_term]["path_length"] = $path;
    }

    return $ancestors;

  }


  /**
   * Fetches all cvterms associated with a chado record.
   * Terms can be associated with a record in the following ways:
   *
   *  - Type ID in record table
   *  - Property type in recordprop table
   *  - Property cvalue in recordprop table
   *
   * @param $base_table - the chado base table (ie feature)
   * @param $record_id
   *
   * @return array|null  A nested array where each cvterm ID is a key, and the
   *   value is the relationship to the record. (type, prop_type, or
   *   prop_value)
   */

  public function get_all_associated_cvterms($base_table, $record_id) {
    $out = [];

    $chado_table = "chado." . $base_table;
    $prop_table = "chado." . $base_table . "prop";
    if ($base_table != 'feature') {
      return NULL;
    }

    if (db_field_exists($chado_table, "type_id")) {
      $query = db_select($chado_table, "CT");
      $query->fields("CT", ["type_id"])
        ->condition("feature_id", $record_id);
      $type = $query->execute()->fetchObject();
    }

    if ($type->type_id) {
      $out[$type->type_id] = "type";
    }

    $query = db_select($prop_table, "P");
    $query->fields("P", ["type_id"])
      ->condition("feature_id", $record_id);
    $prop_types = $query->execute()->fetchAll();

    foreach ($prop_types as $term) {
      $out[$term->type_id] = "prop_type";
    }

    if (db_field_exists($prop_table, "cvalue_id")) {
      $query = db_select($prop_table, "P");
      $query->fields("P", ["type_id"])
        ->condition("feature_id", $record_id);
      $prop_cvalues = $query->execute()->fetchAll();

      foreach ($prop_cvalues as $term) {
        $out[$term->type_id] = "prop_value";
      }
    }

    return $out;
  }

  /**
   * Gets array of all Tripal bundle entities
   *
   * @return array|bool
   */
  private function get_all_bundles() {
    $bundles = [];
    // Get all bundle names to cycle through.
    $all_bundles = db_select('tripal_bundle', 'tb')
      ->fields('tb', ['name'])
      ->execute()
      ->fetchAll();
    foreach ($all_bundles as $bundle_name) {
      // Get the bundle object.
      $bundle = tripal_load_bundle_entity(['name' => $bundle_name->name]);
      if (!$bundle) {
        tripal_report_error('tripal', TRIPAL_ERROR, "Unrecognized bundle name '%bundle'.",
          ['%bundle' => $bundle_name]);

        return FALSE;
      }
      array_push($bundles, $bundle);
    }

    return $bundles;
  }

}