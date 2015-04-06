(function ($) {
Drupal.behaviors.commerce_my_delivery_times = {
  attach: function (context) {
    for (var id in Drupal.settings.datePopup) {
    	Drupal.settings.datePopup[id].settings.beforeShowDay = nationalDays;
    }
  }
};

function nationalDays(date) {
	var disabledDays = Drupal.settings.delivery_dates.date_data;
	var m = date.getMonth(), d = date.getDate(), y = date.getFullYear();
	for (i = 0; i < disabledDays.length; i++) {
		if($.inArray((m+1) + '-' + d + '-' + y,disabledDays) != -1 || new Date() > date) {
			return [false];
		}
	}
	return [true];
}
})(jQuery);




jQuery(document).ready(function($) {
    jQuery.noConflict();
    
    jQuery(".cp_checkout").click(function(event)
    {
        var cts_id = jQuery('input:radio[name=cts_id_checkout]:checked').val(); 
        alert(cts_id);
        
        
        $.ajax({
            type: "POST",
            url: "/mycommerce/set-delivery-time-slot",
            data: "delivery_time_slot_id=" + cts_id,
            success: function(html) {
                console.log(html);
                
                var return_data = html.split('|');
                var cts_id = return_data[0];
                var click_and_collect_slot_date = return_data[1];
                var remaining_bucket_size = return_data[2];
                var slot_id = return_data[3];
                var previous_cts_id = return_data[4];
                var previous_bucket_size = return_data[5];
                var existing_field_check = return_data[6];
                
                //updating current time slot
                var span_id = cts_id+'-'+click_and_collect_slot_date;
                if(existing_field_check == 0) {
                    // if its for first time db entry
                    jQuery('span#'+span_id+'_cp_bucket').html(remaining_bucket_size);
                } else {
                    jQuery('span#'+slot_id+'_cp_bucket').html(remaining_bucket_size);
                }
                if(span_id!=slot_id) {
                    jQuery('span#'+span_id+'_cp_bucket').attr('id',slot_id+'_cp_bucket');
                }
                if(previous_cts_id != 0) {
                    // updating previous time slot
                    jQuery('span#'+previous_cts_id+'_cp_bucket').html(previous_bucket_size);
                }
            }
        });
    
    });
    
});