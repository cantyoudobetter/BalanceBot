
var listObject = {
    deviceid : null,
    devicename : null
}
var data = []
var updateInterval = 1000;
var totalPoints = 100;
var now = new Date().getTime();
var bat = 0;
var activitydata =[];
var currentts = 0;
var soundPlayed = 0;


var activityoptions = {
    series: { shadowSize: 1 },
    lines: { fill: true, fillColor: { colors: [ { opacity: .2 }, { opacity: 0.1 } ] }},
    yaxis: {
        min: 0,
        max: 100,                          
        tickFormatter: function (v, axis) {
            if (v % 10 == 0) {
                return v + "%";
            } else {
                return "";
            }
        },
        axisLabel: "Battery Level",
        axisLabelUseCanvas: true,
        axisLabelFontSizePixels: 12,
        axisLabelFontFamily: 'Verdana, Arial',
        axisLabelPadding: 6
    },
    xaxis: {
        mode: "time",
        tickSize: [1, "hour"],
        timeformat: "%I:%M%p",
        axisLabel: "Time",
        axisLabelUseCanvas: true,
        axisLabelFontSizePixels: 12,
        axisLabelFontFamily: 'Verdana, Arial',
        axisLabelPadding: 10
    }
}; 



$(document).on('pagebeforeshow', '#devices', function(){       
    $('#device-listview li a').each(function(){
        var elementID = $(this).attr('id');
        var devicenametmp = $('#devicename'+elementID).text();      
        $(document).on('click', '#'+elementID, function(event){  
            if(event.handled !== true)
            {
                listObject.deviceid = elementID; 
                listObject.devicename = devicenametmp;
                $.mobile.changePage( "#menu", { transition: "slide"} );
                event.handled = true;
            }              
        });
    });
});

$(document).on('pagebeforeshow', '#menu', function(){       
    //$('#title-device').html('<h1>' + deviceid + '<h1>');
    $('#title-device').text(listObject.devicename);
    updateBattery();
    updateAlerts();
    updateFall();
    function updateBattery() { 
        $.getJSON( "./api/v1/lastbattery/"+listObject.deviceid, function( datatemp ) {
            var val = datatemp.battery;
            $("#batterylife-amount").css("width",val*3);
            $("#menu-batterylife-val").text(val+'%');
            $("#battery-batterylife-val").text(val+'%');

        });
        setTimeout(updateBattery, updateInterval*100);
    } 
    function updateAlerts() { 
        $.getJSON( "./api/v1/buttonpresses/"+listObject.deviceid, function( datatemp ) {
            if (datatemp.dayCount > 0 && !soundPlayed) { 
                $.playSound('assets/img/btnalert'); 
                soundPlayed = 1;
            }
            if (datatemp.dayCount > 0 ) $('#menu-alert-icon').trigger('mouseover');
            else $('#menu-alert-icon').trigger('mouseout');
            $('#menu-button-val').text(datatemp.dayCount);
            $('#alert-presses-today').text(datatemp.dayCount);
            $('#alert-presses-time').text(datatemp.lastPressDate);

        });
        setTimeout(updateAlerts, updateInterval*100);
    } 
    function updateFall() { 
        $.getJSON( "./api/v1/fallcount/"+listObject.deviceid, function( datatemp ) {
            if (datatemp.dayCount > 0 && !soundPlayed) { 
                $.playSound('assets/img/btnalert'); 
                soundPlayed = 1;
            }
            if (datatemp.dayCount > 0 ) $('#menu-fall-icon').trigger('mouseover');
            else $('#menu-fall-icon').trigger('mouseout');
            $('#fall-button-val').text(datatemp.dayCount);
            $('#falls-today').text(datatemp.dayCount);
            $('#fall-last-time').text(datatemp.lastFallDate);


        });
        setTimeout(updateFall, updateInterval*100);
    } 

    
});

$(document).on('pagebeforeshow', '#alert', function(){       
        $( ".al" ).remove();
        $.getJSON( "./api/v1/alertlist/"+listObject.deviceid, function( datatemp ) {
            $.each(datatemp, function(idx, obj) {
                $( "#alert-log-list" ).append( "<li class='al'>"+obj+"</li>" ).listview('refresh');
            });

        });


});

$(document).on('pagebeforeshow', '#fall', function(){       
        $( ".al" ).remove();
        $.getJSON( "./api/v1/falllist/"+listObject.deviceid, function( datatemp ) {
            $.each(datatemp, function(idx, obj) {
                $( "#fall-log-list" ).append( "<li class='al'>"+obj+"</li>" ).listview('refresh');
            });
        });
});

(function($){

  $.extend({
    playSound: function(){
      return $("<embed src='"+arguments[0]+".mp3' hidden='true' autostart='true' loop='false' class='playSound'>" + "<audio autoplay='autoplay' style='display:none;' controls='controls'><source src='"+arguments[0]+".mp3' /><source src='"+arguments[0]+".ogg' /></audio>").appendTo('body');
    }
  });

})(jQuery);


$(document).on('pagebeforeshow', '#history', function(){    
    updateActivityHistory();

    $("#48,#24,#6,#11").click(function () {
        var hours = parseInt($(this).attr('id'));
        var ticks = 0;
        var ticktype = "";
        var format = "%I:%M%p";
        switch(hours) {
            case 48:
                ticks = 12;
                ticktype = "hour";
                format = "%m/%d %I:%M%p";
                break;
            case 24:
                ticks = 6;
                ticktype = "hour";
                break;
            case 6:
                ticks = 2;
                ticktype = "hour";
                break;
            case 11:
                ticks = 120;
                ticktype = "minute";
                break;
            default:
                break;
        }
        $.plot("#activitychart", activitydata, {
                series: { shadowSize: 1 },
                lines: { fill: true, fillColor: { colors: [ { opacity: .2 }, { opacity: 0.1 } ] }},
                yaxis: {
                    min: 0,
                    max: 100,                          
                    tickFormatter: function (v, axis) {
                        if (v % 10 == 0) {
                            return v + "%";
                        } else {
                            return "";
                        }
                    },
                    axisLabel: "Battery Level",
                    axisLabelUseCanvas: true,
                    axisLabelFontSizePixels: 12,
                    axisLabelFontFamily: 'Verdana, Arial',
                    axisLabelPadding: 6
                },
                xaxis: {
                    mode: "time",
                    min: (currentts - hours*60*60*1000),
                    max: currentts,
                    tickSize: [ticks, ticktype],
                    timeformat: format,
                    axisLabel: "Time",
                    axisLabelUseCanvas: true,
                    axisLabelFontSizePixels: 12,
                    axisLabelFontFamily: 'Verdana, Arial',
                    axisLabelPadding: 10
                }
            })
    });


    function updateActivityHistory() { 
        $.getJSON( "./api/v1/activitylevel1day/"+listObject.deviceid, function( datatemp ) {
            var datafinal = [];
            datatemp.forEach( function (value, index, ar) {
                var dt = new Date();
                var off = dt.getTimezoneOffset();
                var dtpassed = value[0];
                var dtLocal = dtpassed - dt.getTimezoneOffset()*60;
                var arr = [dtLocal*1000,value[1]];
                datafinal.push(arr);
            });
            activitydata = [
                { label: "Activity Level", data: datafinal, color: "#418bca" }
            ];
            $.plot($("#activitychart"), activitydata, activityoptions);
        });
    }


}); 