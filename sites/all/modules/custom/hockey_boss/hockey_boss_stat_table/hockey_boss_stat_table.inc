<?php
/**
  @file
  @brief Defines the stat tables.
*/

function hockey_boss_stat_table_edit_pps ($user) {
  return true;
}

/**
  @brief Accepts a player node and returns a simple
    table form that represents the player's
    performance stats table.
  @param $player (node) the player node.
  @return (array) the stats table form.
*/
function hockey_boss_stat_table_pps ($player) {
  module_load_include ('inc', 'hockey_boss');
  module_load_include ('inc', 'hockey_boss_stat');
  module_load_include ('inc', 'stat');

  $stats = stat_get_stats_by_subject (hockey_boss_stat_PPS_TYPE, $player->nid);

  $data = array (
    'columns' => array (
      array (
        'column_name' => t ('Year'),
        'weight'      => 10
    )), 
    'sort_column'     => t ('Year'),
    'access_function' => 'hockey_boss_stat_table_edit_pps'
  );  
  $stat_years = array (); 
  foreach ($stats as $stat) {
    $stat_year = date ('Y', strtotime (stat_get_date ($stat)));
    if (!in_array ($stat_year, $stat_years)) {
      $data ['fields'][] = array (
        'column_name'   => t ('Year'),
        'row_name'      => $stat_year,
        'value'         => $stat_year
      );  
      $stat_years [] = $stat_year;
    }   
    $stat_uri   = node_uri ($stat);
    $stat_unit  = stat_get_unit ($stat);
    $stat_value = stat_get_value ($stat);
    $data ['fields'][] = array (
      'column_name'   => $stat->title,
      'row_name'      => $stat_year,
      'stat'          => $stat->nid,
      'value'         => $stat_value,
    );
  }
  return $data;
}
