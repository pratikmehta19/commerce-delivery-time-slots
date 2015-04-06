<?php

require_once 'slot_configuration.inc';

function edit_delivery_times_config_form($form, $form_state, $dtsm_id) {

  $delivery_times_master = db_select('delivery_time_slots_master', 'dtsm')
          ->fields('dtsm', array('delivery_slot_date', 'no_of_delivery_slot'))
          ->condition('dtsm.dtsm_id', $dtsm_id)
          ->execute()
          ->fetchAssoc();

  $delivery_slot_details = db_select('delivery_time_slots', 'dts')
          ->fields('dts')
          ->condition('dts.dtsm_id', $dtsm_id)
          ->execute()
          ->fetchAll();
  
  $dts_id = array();
  $slot_start_time = array();
  $slot_end_time = array();
  $slot_bucket_size = array();
  $slot_threshold_time_accepting_order = array();

  foreach ($delivery_slot_details as $slot_details) {
    $dts_id[] = $slot_details->dts_id;
    $slot_start_time[] = $slot_details->slot_start_time;
    $slot_end_time[] = $slot_details->slot_end_time;
    $slot_bucket_size[] = $slot_details->slot_bucket_size;
    $slot_threshold_time_accepting_order[] = $slot_details->slot_threshold_time_accepting_order;
  }
  
  $form['dtsm_id'] = array(
      '#type' => 'hidden',
      '#default_value' => $dtsm_id,
  );
  
if($delivery_times_master['delivery_slot_date'] != 0) {
  $form['delivery_slot_date'] = array(
      '#type' => 'textfield',
      '#title' => t('Delivery Slot Date'),
      '#attributes' => array('readonly' => 'readonly'),
      '#default_value' => $delivery_times_master['delivery_slot_date'],
  );
  $form['slot_type'] = array(
  	  '#type' => 'hidden',
  	  '#default_value' => 'individual',	
  );
} else {
  $form['delivery_slot_date'] = array(
      '#type' => 'hidden',
      '#default_value' => 0,
  );
  $form['slot_type'] = array(
  		'#type' => 'hidden',
  		'#default_value' => 'master',
  );
}
  
  $form['#tree'] = TRUE;

  $form['no_of_delivery_slot'] = array(
      '#type' => 'select',
      '#default_value' => $delivery_times_master['no_of_delivery_slot'],
      '#title' => t('No. of Delivery Slots'),
      '#options' => array(
          '1' => '1',
          '2' => '2',
          '3' => '3',
          '4' => '4',
      ),
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

  if (empty($form_state['values']['no_of_delivery_slot'])) {
    $no_of_rows = $delivery_times_master['no_of_delivery_slot'];
  } 
  else {
    $no_of_rows = $form_state['values']['no_of_delivery_slot'];
  }

  for ($i = 1; $i < $no_of_rows + 1; $i++) {
	$form['slot_data'][$i] = array(
        '#type' => 'fieldset',
        '#title' => check_plain(t('Slot ' . $i)),
        '#prefix' => '<div id="slot_data">',
        '#suffix' => '</div>',
    );
	/*
	$form['slot_data'][$i]['dts_id'] = array(
        '#type' => 'hidden',
        '#title' => check_plain(t('Slot ' . $i)),
        '#default_value' => $dts_id[$i - 1],
    );
    */
	$form['slot_data'][$i]['start_time'] = array(
        '#type' => 'textfield',
        '#title' => t('Start Time'),
        '#required' => TRUE,
        '#default_value' => $slot_start_time[$i - 1],
        '#description' => t('Enter Start Time of Slot in 24 Hours Format i.e 10 or 20'),
    );
    $form['slot_data'][$i]['end_time'] = array(
        '#type' => 'textfield',
        '#title' => t('End Time'),
        '#required' => TRUE,
        '#default_value' => $slot_end_time[$i - 1],
        '#description' => t('Enter End Time of Slot in 24 Hours Format i.e 10 or 20'),
    );
    $form['slot_data'][$i]['bucket_size'] = array(
        '#type' => 'textfield',
        '#title' => t('Delivery Capacity'),
        '#required' => TRUE,
        '#default_value' => $slot_bucket_size[$i - 1],
        '#description' => t('Enter maximum no. of order acceptance for this slot'),
    );
    $form['slot_data'][$i]['threshold_time_accepting_order'] = array(
        '#type' => 'textfield',
        '#title' => t('Threshold Time for Accepting Order'),
        '#required' => TRUE,
        '#default_value' => $slot_threshold_time_accepting_order[$i - 1],
        '#description' => t('Enter threshold time for order acceptance'),
    );
  }  // end for

  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => 'SUBMIT',
  );

  $form['#validate'][] = 'delivery_slot_config_validation';

  return $form;
}



function edit_delivery_times_config_form_submit($form, &$form_state) {
  $dtsm_id = $form_state['values']['dtsm_id'];
  db_delete('delivery_time_slots_master')->condition('dtsm_id', $dtsm_id)->execute();
  db_delete('delivery_time_slots')->condition('dtsm_id', $dtsm_id)->execute();
  delivery_times_config_form_submit($form, $form_state);

}


function delete_delivery_time_slots($dtsm_id) {
  db_delete('delivery_time_slots_master')->condition('dtsm_id', $dtsm_id)->execute();
  db_delete('delivery_time_slots')->condition('dtsm_id', $dtsm_id)->execute();
  drupal_set_message(t("Delivery Slot has been deleted Successfully."));
  drupal_goto("admin/commerce/delivery-times/update-configuration");
}



?>
