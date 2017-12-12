<?php
/**
 * Updates all existing url aliases for an entity.
 *
 * @param $bundle_id
 * @param $update
 * @param $type
 */
function tripal_update_all($bundle_id, $update, $type) {
  // Load all the entity_ids.
  $entity_table = 'chado_' . $bundle_id;
  $result = db_select($entity_table, 'e')
    ->fields('e', ['entity_id'])
    ->orderBy('entity_id', 'ASC')
    ->execute();
  $num_entities = $result->rowCount();

  // Parse the $update variable for tokens and load those tokens.
  preg_match_all("/\[[^\]]*\]/", $update, $bundle_tokens);
  $field_ids = [];
  foreach ($bundle_tokens as $index => $token) {
    if ($index > 0) {
      $field = field_info_instance('TripalEntity', $bundle_tokens[1], $bundle_id);
      $field_ids[] = $field['field_id'];
    }
  }

  $page = 0;
  $i = 1;
  while ($entities = tripal_update_all_get_next_chunk($page, $entity_table, $num_entities)) {
    $page++;
    // Pull items out one at a time.
    while ($entity = $entities->fetchObject()) {
      // Load the entity with only the specified fields
      $arg = tripal_load_entity('TripalEntity', [$entity->entity_id], FALSE, $field_ids);
      if ($type == 'alias') {
        if (!empty($arg)) {
          if (is_array($arg)) {
            $ent = reset($arg);
          }
          // Get the entity controller and clear the cache if requested (default).
          $ec = entity_get_controller('TripalEntity');
          $ec->setAlias($ent, $update);
        }
      }
      elseif ($type == 'title') {
        if (!empty($arg)) {
          if (is_array($arg)) {
            $ent = reset($arg);
          }
          $ec = entity_get_controller('TripalEntity');
          $ec->setTitle($ent, $update);
        }
      }
      $i++;
      // Check if 50 items have been updated, if so print message.
      if ($i <= $num_entities) {
        print $i . "/" . $num_entities . " entities have been updated.\r";
      }
    }
  }
}

/**
 * Get the next chunk if available.
 *
 * @param $page
 * @param $table
 * @param $total_count
 *
 * @return bool
 */
function tripal_update_all_get_next_chunk($page, $table, $total_count) {
  $offset = $page * 1000;
  $limit = 1000;

  if ($offset >= $total_count) {
    return FALSE;
  }

  return db_select($table, 'e')
    ->fields('e', ['entity_id'])
    ->orderBy('entity_id', 'ASC')
    ->range($offset, $limit)
    ->execute();
}