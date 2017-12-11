<?php
/**
 * Updates all existing url aliases for an entity.
 *
 * @param $bundle_id
 * @param $update
 * @param $type
*/
function tripal_update_all($bundle_id, $update, $type) {
  //Load all the entity_ids.
  $entity_table = 'chado_'.$bundle_id;
  $entities = db_select($entity_table, 'e')
  ->fields('e', array('entity_id'))
  ->execute()->fetchAll();
  $num_entities = count($entities);
  //Parse the $update variable for tokens and load those tokens.
  preg_match_all("/\[[^\]]*\]/", $update, $bundle_tokens);
  $i = 1;
  $field_ids = [];
  foreach ($bundle_tokens as $index => $token) {
    if($index > 0) {
      $field = field_info_instance('TripalEntity', $bundle_tokens[1], $bundle_id);
      $field_ids[] = $field['field_id'];
    }
  }

  //Pull items out one at a time.
  foreach($entities as $entity) {
    $arg = tripal_load_entity('TripalEntity', [$entity->entity_id], FALSE, $field_ids);
    if ($type == 'alias') {
      if (!empty($arg)) {
        if(is_array($arg)){
          $ent = reset($arg);
        }
        // Get the entity controller and clear the cache if requested (default).
        $ec = entity_get_controller('TripalEntity');
        $ec->setAlias($ent, $update);
      }
    }
    elseif ($type == 'title') {
      if (!empty($arg)) {
        if(is_array($arg)){
          $ent = reset($arg);
        }
        $ec = entity_get_controller('TripalEntity');
        $ec->setTitle($ent, $update);
      }
    }
    $i++;
    // Check if 50 items have been updated, if so print message.
    if ($i <= $num_entities){
      print $i."/".$num_entities." entities have been updated.\r";
    }
  }
}

