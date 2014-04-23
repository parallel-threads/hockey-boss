<?php
/**
  @file
  @brief Defines the stat tables.
*/

/// Represents player performance stat tables. 
class PPSTable extends SimpleTable {
  /**
    The node id of the player whose stats are
    displayed in this table.
  */
  public $player_nid = null;

  /// Returns a player performance stat table.
  public function __construct ($player_nid) {
    parent::__construct ();
    
    $this->player_nid = $player_nid;

    // set row names.
    $start_year = '2012';
    $current_year = date ('Y');
    for ($year = $start_year; $year <= $current_year; $year ++) {
      $this->row_names [] = $year;
    }

    // set columns.
    $this->columns = $this->create_columns (
      array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Year'));

    $editable = $this->editable ();
    $this->submittable       = $editable;
    $this->sortable          = !$editable;
    $this->sort_column_name  = 'Year';
    $this->label_column_name = 'Year';
  }

  /// Returns true iff the current user has permission to edit this table.
  public static function editable () {
    return false;
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'stat');
    module_load_include ('inc', 'hockey_boss_stat');

    $field_width = 5;

    $field_stat_name = $column_name;
    $field_stat_year = $row_name;

    $field_stat = null;
    $stats = stat_get_stats_by_subject_nid (hockey_boss_stat_PPS_TYPE, $this->player_nid);
    foreach ($stats as $stat) {
      $stat_year = stat_get_year ($stat);
      if ($stat->title == $field_stat_name &&
          $stat_year == $field_stat_year) {
        $field_stat = $stat;
        break;
      }      
    }


    if (is_null ($field_stat)) {
      $stat_date  = simple_field_create_date_field_value ($field_stat_year, 0, 0);
      $stat_date  = $stat_date ['value'];
      $stat_type  = hockey_boss_stat_pps_type ();
      $field_stat = stat_create_stat_node ($stat_type, $field_stat_name, '', '', $stat_date, $this->player_nid);
    }

    $stat_value = stat_get_value ($field_stat);
    if ($this->editable ()) {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => array (
          '#type' => 'container',
          "stat_value_$stat->nid" => array (
            '#type'               => 'textfield',
            '#default_value'      => $stat_value,
            '#size'               => $field_width
      )));
    } else {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => $stat_value
      );
    }
  }

  /// The submit callback.
  public static function process_table_form ($form, $form_state) {
    module_load_include ('inc', 'stat');
    foreach ($form_state ['input'] as $key => $value) {
      if (strpos ($key, 'stat_value_') === 0) {
        $stat_nid = substr ($key, 11);
        $stat = node_load ($stat_nid);
        if (is_null ($stat)) {
          continue;
        }
        stat_set_value ($stat, $value);
        node_save ($stat);
      }
    }
    drupal_set_message ('Stats updated.');
  }
}

/// Represents player game stat tables.
class PGSTable extends SimpleTable {
  /**
    The node id of the player whose stats are
    displayed in this table.
  */
  public $player = null;

  /// Returns a player game stat table.
  public function __construct ($player) {
    parent::__construct ();

    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');
    module_load_include ('inc', 'stat');

    $this->player = $player;

    // I. set row names.
    $player_team_nid = hockey_boss_get_player_team_nid ($player);
    $games = hockey_boss_get_game_nodes ();
    $game_nids = array ();
    foreach (hockey_boss_get_game_nodes () as $game) {
      $game_team_nids = hockey_boss_get_game_team_nids ($game);
      if ($player_team_nid == $game_team_nids [0] ||
          $player_team_nid == $game_team_nids [1]) {
        $game_nids [] = $game->nid;
      }
    }

    $this->row_names = $game_nids;

    // II. set columns.
    $column_index = 0;
    $column_names = array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Date', 'Game');
    foreach ($column_names as $column_name) {
      $this->columns [] = array (
        'name'   => $column_name,
        'label'  => $column_name,
        'weight' => $column_index
      );
     $column_index ++;
    }

    $editable = $this->editable ();
    $this->submittable      = $editable;
    $this->sortable         = !$editable;
    $this->sort_column_name = 'Date';
  }

  /// Returns true iff the current user has permission to edit this table.
  public static function editable () {
    return false;
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'stat');
    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');

    $player = $this->player;

    $field_stat_name = $column_name;
    $field_stat_game_nid  = $row_name;
    $game_node = node_load ($field_stat_game_nid);
    if (!$game_node) {
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    if ($column_name === 'Game') {
      return array (
        'id'            => null,
        'value'         => $game_node->title,
        'display_value' => $game_node->title
      );
    }
    if ($column_name === 'Date') {
      $game_date = date_format (date_create (hockey_boss_get_game_date ($game_node)), 'Y-m-d');
      return array (
        'id'            => null,
        'value'         => $game_date,
        'display_value' => $game_date
      );
    }

    $field_stat = null;
    $stats = stat_get_stats_by_subject_nid (hockey_boss_stat_PGS_TYPE, $player->nid);
    foreach ($stats as $stat) {
      $stat_game_nid = hockey_boss_stat_pgs_get_game ($stat);
      if ($stat->title === $field_stat_name &&
          $stat_game_nid === $field_stat_game_nid) {
        $field_stat = $stat;
        break;
      }
    }

    if (is_null ($field_stat)) {
      $stat_date = $game_node->{$game_node->type . '_date'}['und'][0]['value']; 
      $stat_type = hockey_boss_stat_pgs_type ();
      $field_stat = stat_create_stat_node ($stat_type, $field_stat_name, '', '', $stat_date, $player->nid);
      hockey_boss_stat_pgs_set_game ($field_stat, $game_node->nid);
      node_save ($field_stat);
    }

    $stat_value = stat_get_value ($field_stat);
    if ($this->editable ()) {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => array (
          '#type' => 'container',
          "stat_value_$stat->nid" => array (
            '#type'               => 'textfield',
            '#default_value'      => $stat_value,
            '#size'               => $field_width
      )));
    } else {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => $stat_value
      );
    } 
  }

  /// The submit callback.
  public static function process_table ($form, $form_state) {
    module_load_include ('inc', 'stat');
    foreach ($form_state ['input'] as $key => $value) {
      if (strpos ($key, 'stat_value_') === 0) {
        $stat_nid = substr ($key, 11);
        $stat = node_load ($stat_nid);
        if (is_null ($stat)) {
          continue;
        }
        stat_set_value ($stat, $value);
        node_save ($stat);
      }
    }
    drupal_set_message ('Stats updated.');
  }
}

/// Represents the team roster tables.
class TRTable extends SimpleTable {
  /**
    The team profile node of the team whose players
    are displayed in this table.
  */
  public $team = null;

  /// Returns a team roster table.
  public function __construct ($team) {
    parent::__construct ();

    module_load_include ('inc', 'hockey_boss');

    $this->team = $team;

    // set row names.
    foreach ((array) hockey_boss_get_nodes (hockey_boss_PLAYER_PROFILE_TYPE) as $player_nid) {
      $player = node_load ($player_nid);
      if (hockey_boss_get_player_team_nid ($player) == $team->nid) {
        $this->row_names [] = $player->nid;
      }
    }

    // set columns.
    $this->columns = $this->create_columns (
      array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Position', 'Name'));

    $this->sort_column_name = 'Name';
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'hockey_boss_stat');
    module_load_include ('inc', 'stat');

    $player_nid = $row_name;
    $player = node_load ($player_nid);
    if (!$player) {
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    if ($column_name === 'Name') {
      $player_node_uri = node_uri ($player);
      $player_profile_link = l ($player->title, $player_node_uri ['path']);
      return array (
        'id'            => null,
        'value'         => $player->title,
        'display_value' => $player_profile_link
      );
    } 
    if ($column_name === 'Position') {
      // To Be Implemented
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    $stat_name = $column_name;

    $stat = null;
    $current_year = date ('Y');
    foreach ((array) stat_get_stats_by_subject_nid (hockey_boss_stat_PPS_TYPE, $player->nid) as $_stat) {
      if ($_stat->title == $stat_name && stat_get_year ($_stat) === $current_year) {
        $stat = $_stat;
        break;
      }
    }

    if (is_null ($stat)) {
      $stat_date  = simple_field_create_date_field_value ($current_year, 0, 0);
      $stat_date  = $stat_date ['value'];
      $stat_type  = hockey_boss_stat_pps_type ();
      $stat = stat_create_stat_node ($stat_type, $stat_name, '', '', $stat_date, $player->nid);
    }

    $stat_value = stat_get_value ($stat);
    $field_width = 3;
    return array (
      'id'            => null,
      'value'         => $stat_value,
      'display_value' => $stat_value
    );
  }
}

/// Represents the player comparison tables.
class PCTable extends SimpleTable {

  /// Returns a player comparison table.
  public function __construct ($player_nids) {
    parent::__construct ();

    // set row names.
    foreach ((array) $player_nids as $player_nid) {
      $player = node_load ($player_nid);
      if ($player) {
        $this->row_names [] = $player_nid;
      }
    }

    // set columns.
    $this->columns = $this->create_columns (
      array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Position', 'Name'));

    $this->sort_column_name = 'Name';
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    $player_nid = $row_name;
    $player = node_load ($player_nid);
    if (!$player) {
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    if ($column_name === 'Name') {
      $player_node_uri = node_uri ($player);
      $player_profile_link = l ($player->title, $player_node_uri ['path']);
      return array (
        'id'            => null,
        'value'         => $player->title,
        'display_value' => $player_profile_link
      );
    }  
    if ($column_name === 'Position') {
      // To Be Implemented
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    $stat_name = $column_name;

    $stat = null;
    $current_year = date ('Y');
    foreach ((array) stat_get_stats_by_subject_nid (hockey_boss_stat_PPS_TYPE, $player->nid) as $_stat) {
      if ($_stat->title == $stat_name && stat_get_year ($_stat) === $current_year) {
        $stat = $_stat;
        break;
      }
    }

    if (is_null ($stat)) {
      $stat_date  = simple_field_create_date_field_value ($current_year, 0, 0);
      $stat_date  = $stat_date ['value'];
      $stat_type  = hockey_boss_stat_pps_type ();
      $stat = stat_create_stat_node ($stat_type, $stat_name, '', '', $stat_date, $player->nid);
    }

    $stat_value = stat_get_value ($stat);
    $field_width = 3;
    return array (
      'id'            => null,
      'value'         => $stat_value,
      'display_value' => $stat_value
    );
  }
}
