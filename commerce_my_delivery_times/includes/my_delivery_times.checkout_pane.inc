<?php

function my_delivery_times_checkout_form($form, &$form_state, $checkout_pane, $order) {

  $no_of_forward_days = variable_get('no_of_forward_days');
  $count = $no_of_forward_days;
   
  $current_date = date("Y-m-d");
  $aDays = array();
  for ($i = 1; $i <= $count; $i++) {
    $sCurrentDate = date("Y-m-d", strtotime("+" . $i . " day", strtotime($current_date)));
    $aDays[] = $sCurrentDate;
  }

  $delivery_slot_display = array();
  foreach ($aDays as $i => $date) {

  $holiday_data = db_select('delivery_holiday_list', 'dhl')
            ->fields('dhl', array('holiday_date', 'holiday_reason'))
            ->condition('dhl.holiday_date', $date)
            ->execute()
            ->fetchAssoc();
    // if holiday date exist
    if (!empty($holiday_data)) {
      $delivery_slot_display[$date][0] = $holiday_data['holiday_reason'];
    } 
    else {
   	  $query = db_select('delivery_time_slots_master', 'dtsm');
      $query->join('delivery_time_slots', 'dts', 'dtsm.dtsm_id = dts.dtsm_id');
      $query->fields('dtsm');
      $query->fields('dts');
      $query->condition('dtsm.delivery_slot_date', $date);
      $delivery_slot_data = $query->execute()->fetchAll();

      //  dsm($delivery_slot_data);
      if (empty($delivery_slot_data)) {

        $query = db_select('delivery_time_slots_master', 'dtsm');
        $query->join('delivery_time_slots', 'dts', 'dtsm.dtsm_id = dts.dtsm_id');
        $query->fields('dtsm');
        $query->fields('dts');
        $query->condition('dtsm.delivery_slot_date', 0);
        $master_delivery_slot_data = $query->execute()->fetchAll();
        //   dsm("master");
        //  dsm($master_delivery_slot_data);
        foreach ($master_delivery_slot_data as $ds_data => $cp_master) {
          $delivery_slot_display[$date][$ds_data] = array(
              'dtsm_id' => $master_delivery_slot_data[$ds_data]->dtsm_id,
              'dts_id' => $master_delivery_slot_data[$ds_data]->dts_id,
              'delivery_slot_date' => $date,
              'slot_start_time' => $master_delivery_slot_data[$ds_data]->slot_start_time,
              'slot_end_time' => $master_delivery_slot_data[$ds_data]->slot_end_time,
              'slot_bucket_size' => $master_delivery_slot_data[$ds_data]->slot_bucket_size,
              'slot_threshold_time_accepting_order' => $master_delivery_slot_data[$ds_data]->slot_threshold_time_accepting_order,
          );
        }
      } else {
        //   dsm("regular");
        //   dsm($delivery_slot_data);
        foreach ($delivery_slot_data as $ds_data => $ds_regular) {
          $delivery_slot_display[$date][$ds_data] = array(
              'dtsm_id' => $delivery_slot_data[$ds_data]->dtsm_id,
              'dts_id' => $delivery_slot_data[$ds_data]->dts_id,
              'delivery_slot_date' => $date,
              'slot_start_time' => $delivery_slot_data[$ds_data]->slot_start_time,
              'slot_end_time' => $delivery_slot_data[$ds_data]->slot_end_time,
              'slot_bucket_size' => $delivery_slot_data[$ds_data]->slot_bucket_size,
              'slot_threshold_time_accepting_order' => $delivery_slot_data[$ds_data]->slot_threshold_time_accepting_order,
          );
        }
      }
    }
  }

//  dsm($delivery_slot_display);

  $max = array();
  foreach ($delivery_slot_display as $ds_data) {
    $max[] = count($ds_data);
  }
  $max_val = max($max);
//dsm($max_val);

  $date = date("Y-m-d");
  $hours = date('H');
  $mins = date('i');
  $delivery_slot_table = '';
 // $delivery_slot_table .= " <div class='slot_color_indicator' ><img src = '/mycommerce/sites/all/modules/custom/commerce_my_delivery_times/images/checkout-page-delivery-slot_color.png'></div>";
  $delivery_slot_table .= '<div class="top-delivery-slot"><table class="table-bordered table-hover">';

  foreach ($delivery_slot_display as $ds_date => $ds_slot_data) {

    //  dsm($cp_slot_data);
    $count_temp = 0;
    $delivery_slot_table .= '<tr><td class="th">' . date("D, j M", strtotime($ds_date)) . '</td>';
    foreach ($ds_slot_data as $key => $ds_val) {
      // check for if holiday calendar
      if (empty($ds_val['slot_start_time'])) {
        $delivery_slot_table .='<td class="holiday"  align="center" colspan=' . $max_val . '>' . $ds_val . '</td>';
      } else {

        $slot_start_time = $ds_val['slot_start_time'];
        $slot_end_time = $ds_val['slot_end_time'];
        $count_temp++;
        $ds_val_for_id = $ds_val["dts_id"] . '_' . $ds_date . '_' . $slot_start_time . '_' . $slot_end_time . '_' . $ds_val['slot_bucket_size'];

        $nowtime = date("Y-m-d H:i");
        $delivery_slot_date = $ds_val['delivery_slot_date'];
        $slot_date_time = $delivery_slot_date . ' ' . $slot_start_time;
        $slot_threshold_time_accepting_order = $ds_val['slot_threshold_time_accepting_order'];
        $accepting_hours_min = explode('.', $slot_threshold_time_accepting_order);
        $accepting_hours = $accepting_hours_min[0];
        $accepting_min = $accepting_hours_min[1];
        $applicable_time = date('Y-m-d H:i', strtotime($slot_date_time . ' - ' . $accepting_hours . ' hours ' . $accepting_min . ' minute'));
        $remaining_buckets = $ds_val['slot_bucket_size'];
        
        // check for entry in particular date field slot is created or not
        $soft_commit_data_check = db_select('delivery_slot_soft_and_hard_commit', 'sc')
                ->fields('sc')
                ->condition('sc.delivery_time_slot_date', $delivery_slot_date)
                ->condition('sc.slot_start_time', $slot_start_time)
                ->execute()
                ->fetchAssoc();
        if (!empty($soft_commit_data_check)) {

          $slot_id = $soft_commit_data_check['slot_id'];
          $delivery_slot_table .= '<td><input id="' . $slot_id . '" class="cp_checkout" type="radio" name="cts_id_checkout" value="' . $ds_val_for_id . '"';
          if (isset($_SESSION["ds_time_slot_id"])) {
            if ($_SESSION["ds_time_slot_id"] == $slot_id) {
              $delivery_slot_table .= 'checked="checked"';
            }
          }

          if (($nowtime > $applicable_time) || ($soft_commit_data_check['hard_commit_count'] >= $soft_commit_data_check['slot_bucket_size'])) {
            $delivery_slot_table .= 'disabled';
          }
          $time_format = delivery_time_slot_time_format($slot_start_time, $slot_end_time);
          $remaining_size = $soft_commit_data_check['slot_bucket_size'] - $soft_commit_data_check['soft_commit_count'];
          $delivery_slot_table .= '> <label for="' . $soft_commit_data_check["slot_id"] . '">' . $time_format . '(<span class="cp_bucket" id="' . $soft_commit_data_check["slot_id"] . '_cp_bucket">' . $remaining_size . '</span> slots remaining)</label></td>';
        } 
        else {
		 $delivery_slot_table .= '<td><input id="' . $ds_val["dts_id"] . '-' . $ds_date . '" class="cp_checkout" type="radio" name="cts_id_checkout" value="' . $ds_val_for_id . '"';
          if (isset($_SESSION["ds_time_slot_id"])) {
            if ($_SESSION["ds_time_slot_id"] == $ds_val_for_id) {
              $delivery_slot_table .= 'checked="checked"';
            }
          }

          if (($nowtime > $applicable_time)) {
            $delivery_slot_table .= 'disabled';
          }
          $time_format = delivery_time_slot_time_format($slot_start_time, $slot_end_time);
          $delivery_slot_table .= '> <label for="' . $ds_val["dts_id"] . '-' . $ds_date . '">' . $time_format . '(<span class="cp_bucket" id="' . $ds_val["dts_id"] . '-' . $ds_date . '_cp_bucket">' . $remaining_buckets . '</span> slots remaining)</label></td>';
        }
      }
    }

    if (is_array($ds_slot_data[0])) {
      if ($count_temp < $max_val) {
        $diff = $max_val - $count_temp;
        //dsm($diff);
        for ($j = 0; $j < $diff; $j++) {
          $delivery_slot_table .='<td align="center">--</td>';
        }
      }
    }

    $delivery_slot_table .= '</tr>';
  }
  $delivery_slot_table .= '</table></div>';


  $form_pane['checkout_delivery_slot'] = array(
      '#type' => 'markup',
      '#markup' => $delivery_slot_table, 
      '#tree' => true,
  );

  return $form_pane;

}

//
//function click_and_collect_checkout_form_validate(&$form, &$form_state) {
//  
//}

/*
 *  submit function
 */

function my_delivery_times_checkout_form_submit($form, &$form_state, $checkout_pane, $order) {

  $ds_time_slot_id = $_SESSION["ds_time_slot_id"];
//  $order->field_cp_time_slot_id[LANGUAGE_NONE][0]['value'] = $cp_time_slot_id;

  $hard_commit_count = db_select('delivery_slot_soft_and_hard_commit', 'sc')
          ->fields('sc', array('hard_commit_count'))
          ->condition('sc.slot_id', $ds_time_slot_id)
          ->execute()
          ->fetchField();

  $hard_commit_count++;
  
  db_update('delivery_slot_soft_and_hard_commit')
          ->fields(array('hard_commit_count' => $hard_commit_count))
          ->condition('slot_id', $ds_time_slot_id)
          ->execute();
  
  unset($_SESSION["ds_time_slot_id"]);
  
}


function delivery_time_slot_time_format($start_time, $end_time) {
	$delivery_start_time = '';
	if ($start_time == 1) {
		$delivery_start_time = '1 AM';
	}
	elseif ($start_time == 2) {
		$delivery_start_time = '2 AM';
	}
	elseif ($start_time == 3) {
		$delivery_start_time = '3 AM';
	}
	elseif ($start_time == 4) {
		$delivery_start_time = '4 AM';
	}
	elseif ($start_time == 5) {
		$delivery_start_time = '5 AM';
	}
	elseif ($start_time == 6) {
		$delivery_start_time = '6 AM';
	}
	elseif ($start_time == 7) {
		$delivery_start_time = '7 AM';
	}
	elseif ($start_time == 8) {
		$delivery_start_time = '8 AM';
	}
	elseif ($start_time == 9) {
		$delivery_start_time = '9 AM';
	}
	elseif ($start_time == 10) {
		$delivery_start_time = '10 AM';
	}
	elseif ($start_time == 11) {
		$delivery_start_time = '11 AM';
	}
	elseif ($start_time == 12) {
		$delivery_start_time = '12 PM';
	}
	elseif ($start_time == 13) {
		$delivery_start_time = '1 PM';
	}
	elseif ($start_time == 14) {
		$delivery_start_time = '2 PM';
	}
	elseif ($start_time == 15) {
		$delivery_start_time = '3 PM';
	}
	elseif ($start_time == 16) {
		$delivery_start_time = '4 PM';
	}
	elseif ($start_time == 17) {
		$delivery_start_time = '5 PM';
	}
	elseif ($start_time == 18) {
		$delivery_start_time = '6 PM';
	}
	elseif ($start_time == 19) {
		$delivery_start_time = '7 PM';
	}
	elseif ($start_time == 20) {
		$delivery_start_time = '8 PM';
	}
	elseif ($start_time == 21) {
		$delivery_start_time = '9 PM';
	}
	elseif ($start_time == 22) {
		$delivery_start_time = '10 PM';
	}
	elseif ($start_time == 23) {
		$delivery_start_time = '11 PM';
	}
	else {
		$delivery_start_time = '0 AM';
	}



	$delivery_end_time = '';
	if ($end_time == 1) {
		$delivery_end_time = '1 AM';
	}
	elseif ($end_time == 2) {
		$delivery_end_time = '2 AM';
	}
	elseif ($end_time == 3) {
		$delivery_end_time = '3 AM';
	}
	elseif ($end_time == 4) {
		$delivery_end_time = '4 AM';
	}
	elseif ($end_time == 5) {
		$delivery_end_time = '5 AM';
	}
	elseif ($end_time == 6) {
		$delivery_end_time = '6 AM';
	}
	elseif ($end_time == 7) {
		$delivery_end_time = '7 AM';
	}
	elseif ($end_time == 8) {
		$delivery_end_time = '8 AM';
	}
	elseif ($end_time == 9) {
		$delivery_end_time = '9 AM';
	}
	elseif ($end_time == 10) {
		$delivery_end_time = '10 AM';
	}
	elseif ($end_time == 11) {
		$delivery_end_time = '11 AM';
	}
	elseif ($end_time == 12) {
		$delivery_end_time = '12 PM';
	}
	elseif ($end_time == 13) {
		$delivery_end_time = '1 PM';
	}
	elseif ($end_time == 14) {
		$delivery_end_time = '2 PM';
	}
	elseif ($end_time == 15) {
		$delivery_end_time = '3 PM';
	}
	elseif ($end_time == 16) {
		$delivery_end_time = '4 PM';
	}
	elseif ($end_time == 17) {
		$delivery_end_time = '5 PM';
	}
	elseif ($end_time == 18) {
		$delivery_end_time = '6 PM';
	}
	elseif ($end_time == 19) {
		$delivery_end_time = '7 PM';
	}
	elseif ($end_time == 20) {
		$delivery_end_time = '8 PM';
	}
	elseif ($end_time == 21) {
		$delivery_end_time = '9 PM';
	}
	elseif ($end_time == 22) {
		$delivery_end_time = '10 PM';
	}
	elseif ($end_time == 23) {
		$delivery_end_time = '11 PM';
	}
	else {
		$delivery_end_time = '0 AM';
	}
	return $delivery_start_time . ' - ' . $delivery_end_time;
}