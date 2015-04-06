<?php

function delivery_times_config_form($form, $form_state) {


  $dateformat = 'm-d-Y';
  $timestamp = time(); // <- as example, usually this is a db value
  $label = t('Date');
  $dbv = $timestamp;
  $value = NULL;
  // construct date, if db value is empty, default to now
  if (empty($dbv)) {
    $d = new DateTime('@' . $dbv);
    $value = $d->format($dateformat);
  } else {
    $d = new DateTime();
    $value = $d->format($dateformat);
  }

  $date = date('m-d-Y');
  $full_date = array('0' => $date);

  drupal_add_js(drupal_get_path('module', 'commerce_my_delivery_times') . '/js/my_delivery_times.js');

  $settings = array(
      'date_data' => $full_date, // array('8-21-2014', '8-25-2015')
  );
  drupal_add_js(array('delivery_dates' => $settings), 'setting');


  $form['#tree'] = TRUE;

  $form['slot_type'] = array(
      '#title' => t('Slot Type'),
      '#type' => 'radios',
      '#options' => array('master' => t('MASTER'), 'individual' => t('INDIVIDUAL')),
      '#default_value' => 'master',
  );

  $form['delivery_slot_date'] = array(
      '#type' => 'date_popup',
      '#date_timezone' => date_default_timezone(),
      '#date_format' => $dateformat,
      '#date_year_range' => '0:+3',
      '#default_value' => $value,
      '#states' => array(
          'visible' => array(
              ':input[name="slot_type"]' => array('value' => 'individual'),
          ),
          'required' => array(
              ':input[name="slot_type"]' => array('value' => 'individual'),
          ),
      ),
  );

	$max_no_of_slot = variable_get('max_slot_per_day');
	$max_slot = array();
	for($i=1;$i<$max_no_of_slot+1;$i++) {
		$max_slot[$i] = $i;
	}

  $form['no_of_delivery_slot'] = array(
      '#type' => 'select',
      '#title' => t('No. of Delivery Slots'),
      '#options' => $max_slot,
      '#required' => True,
      '#ajax' => array(
          'callback' => 'slot_callback',
          'wrapper' => 'slot_data',
          'effect' => 'fade',
      ),
  );

  $form['slot_data'] = array(
      '#prefix' => '<div id="slot_data">',
      '#suffix' => '</div>',
  );

  if (!empty($form_state['values']['no_of_delivery_slot']) && $form_state['values']['no_of_delivery_slot']) {

    $no_of_rows = $form_state['values']['no_of_delivery_slot'];

    for ($i = 1; $i < $no_of_rows + 1; $i++) {

      $form['slot_data'][$i] = array(
          '#type' => 'fieldset',
          '#title' => check_plain(t('Slot ' . $i)),
          '#prefix' => '<div id="slot_data">',
          '#suffix' => '</div>',
      );

      $form['slot_data'][$i]['start_time'] = array(
          '#type' => 'textfield',
          '#title' => t('Start Time'),
          '#prefix' => '<div class="col1">',
          '#suffix' => '</div>',
          '#required' => TRUE,
          '#description' => t('Enter Start Time of Slot in 24 Hours Format i.e 10 or 20'),
      );
      $form['slot_data'][$i]['end_time'] = array(
          '#type' => 'textfield',
          '#title' => t('End Time'),
          '#prefix' => '<div class="col2">',
          '#suffix' => '</div>',
          '#required' => TRUE,
          '#description' => t('Enter End Time of Slot in 24 Hours Format i.e 10 or 20'),
      );
      $form['slot_data'][$i]['bucket_size'] = array(
          '#type' => 'textfield',
          '#title' => t('Delivery Capacity'),
          '#prefix' => '<div class="col3">',
          '#suffix' => '</div>',
          '#required' => TRUE,
          '#description' => t('Enter maximum no. of order acceptance for this slot'),
      );
      $form['slot_data'][$i]['threshold_time_accepting_order'] = array(
          '#type' => 'textfield',
          '#title' => t('Threshold Time for Accepting Order'),
          '#prefix' => '<div class="col4">',
          '#suffix' => '</div>',
          '#required' => TRUE,
          //    '#attributes' => array('placeholder' => 'Enter threshold time for accepting order i.e. 10.30 '),
          '#description' => t('Enter threshold time for order acceptance'),
      );
      
    }  // end for
  } // end if


  $form['submit'] = array(
      '#type' => 'submit',
      '#validate' => array('delivery_slot_config_validation'),
      '#value' => 'SUBMIT',
  );
  return $form;
}

function slot_callback($form, $form_state) {
  return $form['slot_data'];
}

function delivery_slot_config_validation($form, &$form_state) {

  $slot_type = $form_state['values']['slot_type'];
  $delivery_slot_date = $form_state['values']['delivery_slot_date'];
  $no_of_delivery_slot = $form_state['values']['no_of_delivery_slot'];

  $slot_start_time = array();
  $slot_end_time = array();
  $slot_bucket_size = array();
  $slot_threshold_time_accepting_order = array();
  
  $unique_start_time = array();
  $unique_end_time = array();
  
  for ($i = 1; $i < $no_of_delivery_slot + 1; $i++) {
	
  	if(isset($form_state['values']['slot_data'][$i]['start_time'])) {
    	$slot_start_time[$i] = $form_state['values']['slot_data'][$i]['start_time'];
    	if ((!is_numeric($slot_start_time[$i])) || (!validatetime($slot_start_time[$i]))) {
    		form_set_error("['values']['slot_data'][$i]['start_time", t('Enter Valid start time of slot ' . $i));
    	}
  	}
  	
  	if(isset($form_state['values']['slot_data'][$i]['end_time'])) {
  		$slot_end_time[$i] = $form_state['values']['slot_data'][$i]['end_time'];
  		if ((!is_numeric($slot_end_time[$i])) || (!validatetime($slot_end_time[$i]))) {
  			form_set_error("['values']['slot_data'][$i]['end_time", t('Enter Valid end time of slot ' . $i));
  		}
  	}
  	
  	if(isset($form_state['values']['slot_data'][$i]['bucket_size'])) {
	  	$slot_bucket_size[$i] = $form_state['values']['slot_data'][$i]['bucket_size'];
	  	if (!is_numeric($slot_bucket_size[$i])) {
	  		form_set_error("['values']['slot_data'][$i]['bucket_size", t('Enter Valid Delivery Capacity of slot ' . $i));
	  	}	
  	}
  	
  	if(isset($form_state['values']['slot_data'][$i]['threshold_time_accepting_order'])) {
  		$slot_threshold_time_accepting_order[$i] = $form_state['values']['slot_data'][$i]['threshold_time_accepting_order'];
  		if ((!is_numeric($slot_threshold_time_accepting_order[$i])) || (!validatetime($slot_threshold_time_accepting_order[$i]))) {
	      form_set_error("['values']['slot_data'][$i]['threshold_time_accepting_order", t('Enter Valid threshold time for accepting order of slot ' . $i));
	    }
  	}
  
  	if(isset($form_state['values']['slot_data'][$i]['start_time']) && isset($form_state['values']['slot_data'][$i]['end_time'])) {
	    if ($slot_start_time[$i] >= $slot_end_time[$i]) {
	      form_set_error("['values']['slot_data'][$i]['start_time", "slot start time cant be greater than slot end time for Slot " . $i);
	    }
  	}
  }

  foreach ($slot_start_time as $key => $s_time) {
    if (in_array($s_time, $unique_start_time)) {
      form_set_error("['values']['slot_data'][$key]['start_time", "Start Time already Taken");
    }
    $unique_start_time[] = $s_time;
  }

  foreach ($slot_end_time as $key => $e_time) {
    if (in_array($e_time, $unique_end_time)) {
      form_set_error("['values']['slot_data'][$key]['end_time", "End Time already Taken");
    }
    $unique_end_time[] = $e_time;
  }

  for ($i = 1; $i < $no_of_delivery_slot; $i++) {
  	if(isset($slot_start_time[$i]) && isset($slot_start_time[$i+1])) {
	    if (($slot_start_time[$i + 1] > $slot_start_time[$i]) && ($slot_start_time[$i + 1] < $slot_end_time[$i])) {
	      form_set_error("['values']['slot_data'][$i]['start_time", 'Time slot conflits');
	    }
  	}
  	if(isset($slot_end_time[$i]) && isset($slot_end_time[$i+1])) {
	    if (($slot_end_time[$i + 1] > $slot_start_time[$i]) && ($slot_end_time[$i + 1] < $slot_end_time[$i])) {
	      form_set_error("['values']['slot_data'][$i]['end_time", 'Time slot conflits');
	    }
  	}
  }
// check for individual slot already booked
  if (!empty($form_state['values']['form_id']) && $form_state['values']['form_id'] != 'edit_delivery_times_config_form' && $slot_type == 'individual') {

    $validate_slot = db_select('delivery_time_slots_master', 'dtsm')
            ->fields('dtsm', array('dtsm_id'))
            ->condition('dtsm.delivery_slot_date', $delivery_slot_date)
            ->execute()
            ->fetchAll();

    if ($validate_slot) {
      form_set_error('click_and_collect_slot_date', t('Slot Already booked for that day'));
    }
  }
 // check for master slot if already booked 
	if (!empty($form_state['values']['form_id']) && $form_state['values']['form_id'] != 'edit_delivery_times_config_form' && $slot_type == 'master') {
  
  	$validate_slot = db_select('delivery_time_slots_master', 'dtsm')
  	->fields('dtsm', array('dtsm_id'))
  	->condition('dtsm.delivery_slot_date', 0)
  	->execute()
  	->fetchAll();
  
  	if ($validate_slot) {
  		form_set_error('slot_type', t('Master Slot Already booked, please edit existing slot.'));
  	}
  }
  
}

function delivery_times_config_form_submit($form, &$form_state) {

  $slot_type = $form_state['values']['slot_type'];
  $delivery_slot_date = $form_state['values']['delivery_slot_date'];
  $no_of_delivery_slot = $form_state['values']['no_of_delivery_slot'];

  $slot_start_time = array();
  $slot_end_time = array();
  $slot_bucket_size = array();
  $slot_threshold_time_accepting_order = array();
 
  for ($i = 1; $i < $no_of_delivery_slot + 1; $i++) {
    $slot_start_time[$i] = $form_state['values']['slot_data'][$i]['start_time'];
    $slot_end_time[$i] = $form_state['values']['slot_data'][$i]['end_time'];
    $slot_bucket_size[$i] = $form_state['values']['slot_data'][$i]['bucket_size'];
    $slot_threshold_time_accepting_order[$i] = $form_state['values']['slot_data'][$i]['threshold_time_accepting_order'];
  }

  if ($slot_type == 'master') {
    $delivery_slot_date = '0';
    $slot_type = 1; //master
  } else {
    $slot_type = 0; //individual
  }

  $dtsm_id = db_insert('delivery_time_slots_master')
          ->fields(
                  array(
                      'delivery_slot_date' => $delivery_slot_date,
                      'no_of_delivery_slot' => $no_of_delivery_slot,
                      'delivery_slot_master_individual' => $slot_type,
          ))
          ->execute();


  if (!empty($dtsm_id)) {
    for ($i = 1; $i < $no_of_delivery_slot + 1; $i++) {
      db_insert('delivery_time_slots')
              ->fields(
                      array(
                          'dtsm_id' => $dtsm_id,
                          'slot_start_time' => $slot_start_time[$i],
                          'slot_end_time' => $slot_end_time[$i],
                          'slot_bucket_size' => $slot_bucket_size[$i],
                          'slot_threshold_time_accepting_order' => $slot_threshold_time_accepting_order[$i],
              ))
              ->execute();
    }
    $msg = t("Delivery Time slot has configured successfully");
    drupal_set_message(check_plain($msg));
  }
  drupal_goto("admin/commerce/delivery-times/update-configuration");
}

// View for Updation of Delivery Times slots configuration

function delivery_times_configuration_update() {

  $today = date("Y-m-d");
  $output = '';

  $output .= l('+Configure Slot', 'admin/commerce/delivery-times/slot-configure');
  $output .= "<br/><br/><br/>";

 
$header = array(
      array('data' => 'No of Slots'),
      array('data' => 'Edit'),
      array('data' => 'Delete'),
  );
  $slot_data = db_select('delivery_time_slots_master', 'dtsm')
          ->fields('dtsm', array('dtsm_id', 'no_of_delivery_slot'))
          ->condition('dtsm.delivery_slot_date', 0)
          ->extend('PagerDefault')->limit(9)
          ->range(0, 15)
          ->extend('TableSort')->orderByHeader($header)
          ->execute()
          ->fetchAll();

  if(!empty($slot_data)) {
    
  
 // Master Configuration details
  $output .= "<b>Update Masters</b>";

  $rows = array();
  foreach ($slot_data as $slot_list) {
    $row = array();
    $row [] = $slot_list->no_of_delivery_slot;
    $row [] = l(t('Edit'), 'admin/commerce/delivery-times/edit-slot/' . $slot_list->dtsm_id);
    $row [] = l(t('Delete'), 'admin/commerce/delivery-times/delete-slot/' . $slot_list->dtsm_id, array('attributes' => array('onclick' => 'if(!confirm("Are you sure you want to delete master delivery time slot configuration?")){return false;}')));
    $rows [] = $row;
  }
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= theme('pager');

  }
  
  $header = array(
        array('data' => 'Delivery Time Slot Date'),
        array('data' => 'No of Slots'),
        array('data' => 'Edit'),
        array('data' => 'Delete'),
    );

  $individual_slot_data = db_select('delivery_time_slots_master', 'dtsm')
          ->fields('dtsm', array('dtsm_id', 'delivery_slot_date', 'no_of_delivery_slot'))
          ->condition('dtsm.delivery_slot_date', $today, '>')
          ->extend('PagerDefault')->limit(9)
          ->range(0, 15)
          ->extend('TableSort')->orderByHeader($header)
          ->execute()
          ->fetchAll();

 
  if (!empty($individual_slot_data)) {

     $output .= "<br/><br/>Todays Date : <b>" . $today . "</b>";
    $rows = array();
    foreach ($individual_slot_data as $slot_list) {
      $row = array();
      $row [] = $slot_list->delivery_slot_date;
      $row [] = $slot_list->no_of_delivery_slot;
      $row [] = l(t('Edit'), 'admin/commerce/delivery-times/edit-slot/' . $slot_list->dtsm_id);
      $row [] = l(t('Delete'), 'admin/commerce/delivery-times/delete-slot/' . $slot_list->dtsm_id, array('attributes' => array('onclick' => 'if(!confirm("Are you sure you want to delete delivery time slot?")){return false;}')));
      $rows [] = $row;
    }
    $output .= theme('table', array('header' => $header, 'rows' => $rows));
    $output .= theme('pager');
  } 
  
  if(empty($individual_slot_data) && empty($slot_data)){
    $output .= "<br/>NO SLOT YET CONFIGURED";
  }

  return $output;
}

/*
 *  function validation time
 */
function validatetime($time) {
	$floor_value = floor($time);
	$dec_value = $time - $floor_value;
	if ($dec_value > 0.59) {
		return FALSE;
	} else {
		return TRUE;
	}
}


?>
