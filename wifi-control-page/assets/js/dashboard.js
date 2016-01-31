var data = []
var updateInterval = 1000;
var totalPoints = 100;
var now = new Date().getTime();
var bat = 0;
var currentDeviceId = "1";
var activitydata =[];
var currentts = 0;


 $( document ).ready(function() {
    

      updateBattery();
      updateFall();
      updateActive();
      updateWearerName();
      updateActivity();
      updateActivityHistory();
      updateBatteryHistory();
      updateAlerts();
    

    $( "#guid" ).change(function() {
      currentDeviceId = $( this ).val();
      data=[];
      data.length = 0;
      updateBattery();
      updateFall();
      updateActive();
      updateWearerName();
      updateActivity();
      updateActivityHistory();
      updateBatteryHistory();
      updateAlerts();   
    });


    var dt = new Date();
    currentts = now - dt.getTimezoneOffset()*60*1000;
    function updateBattery() { 
        $.getJSON( "./api/v1/lastbattery/"+currentDeviceId, function( datatemp ) {
            var val = datatemp.battery;
            $('#battery').text(val + '%');
            $('#battery').css('width', val+'%').attr({"aria-valuenow" : val});
            if (val < 15) {
                $('#battery').css('width', val+'%').attr({"class" : "progress-bar-danger six-sec-ease-in-out"});

            } else {
                $('#battery').css('width', val+'%').attr({"class" : "progress-bar-success six-sec-ease-in-out"});

            }


        });
        setTimeout(updateBattery, updateInterval*10);
    } 

    
    function updateFall() { 
        $.getJSON( "./api/v1/fallcount/"+currentDeviceId, function( datatemp ) {
            
            $('#todayFall').text(datatemp.dayCount);
            $('#weekFall').text(datatemp.weekCount);
            $('#monthFall').text(datatemp.monthCount);

        });
        setTimeout(updateFall, updateInterval*10);
    } 
    function updateAlerts() { 
        $.getJSON( "./api/v1/buttonpresses/"+currentDeviceId, function( datatemp ) {
            
            $('#todayAlertCount').text(datatemp.dayCount);
            $('#lastAlertDate').text(datatemp.lastPressDate);

        });
        setTimeout(updateAlerts, updateInterval*100);
    } 

    function updateActive() { 
        $.getJSON( "./api/v1/percentactive/"+currentDeviceId, function( datatemp ) {
            
            $('#restPercent').text(datatemp.restPercent + "%");
            $('#activePercent').text(datatemp.activePercent + "%");

        });
        setTimeout(updateFall, updateInterval*100);
    } 

    function updateWearerName() { 
        $.getJSON( "./api/v1/wearerfordevice/"+currentDeviceId, function( datatemp ) {
            
            $('#device-wearer').text(datatemp.first_name + " " + datatemp.last_name + "'s Dashboard");
            
        });
    } 



    $("#48,#24,#6,#1").click(function () {
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
                    ticks = 1;
                    ticktype = "hour";
                    break;
                case 1:
                    ticks = 20;
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
            });
    });
    if($("#activitychart").length)
    {
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
        function updateActivityHistory() { 
          $.getJSON( "./api/v1/activitylevel1day/"+currentDeviceId, function( datatemp ) {
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
                    $.plot($("#activitychart"), activitydata, batteryoptions);
          });
          
        }

    }



    if($("#batterychart").length)
    {
        var batteryoptions = {
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
                tickSize: [4, "hour"],
                timeformat: "%I:%M%p",
                axisLabel: "Time",
                axisLabelUseCanvas: true,
                axisLabelFontSizePixels: 12,
                axisLabelFontFamily: 'Verdana, Arial',
                axisLabelPadding: 10
            }
        }; 
        function updateBatteryHistory() { 
          $.getJSON( "./api/v1/batterylevel1day/"+currentDeviceId, function( datatemp ) {
                    var datafinal = [];
                    datatemp.forEach( function (value, index, ar) {
                        var dt = new Date();
                        var off = dt.getTimezoneOffset();
                        var dtpassed = value[0];
                        var dtLocal = dtpassed - dt.getTimezoneOffset()*60;
                        var arr = [dtLocal*1000,value[1]];
                        datafinal.push(arr);

                    });
                    dataset = [
                        { label: "Battery Level", data: datafinal, color: "#F89A14" }
                    ];
                    $.plot($("#batterychart"), dataset, batteryoptions);
          });
          //setTimeout(updateActivity, updateInterval);
        }

    }

    if($("#realtimechart").length)
    {
        var options = {
            series: { shadowSize: 1 },
            lines: { fill: true, fillColor: { colors: [ { opacity: 1 }, { opacity: 0.1 } ] }},
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
                axisLabel: "Activity Level",
                axisLabelUseCanvas: true,
                axisLabelFontSizePixels: 12,
                axisLabelFontFamily: 'Verdana, Arial',
                axisLabelPadding: 6
            },
            xaxis: {
                mode: "time",
                tickSize: [60, "second"],
                timeformat: "%I:%M%p",
                axisLabel: "Time",
                axisLabelUseCanvas: true,
                axisLabelFontSizePixels: 12,
                axisLabelFontFamily: 'Verdana, Arial',
                axisLabelPadding: 10
            }
        };

        function updateActivity() { 
          $.getJSON( "./api/v1/lastlevel/"+currentDeviceId, function( datatemp ) {
                    $.each( datatemp, function(key, val) {
                            $.plot($("#realtimechart"), [] , options);
                            if (data.length == 0) {
                                
                                var dt = new Date();
                                var now = dt.getTime() - dt.getTimezoneOffset()*60*1000;

                                var baseNow = (now - updateInterval*totalPoints);
                                while (data.length < totalPoints) { 
                                    
                                    var temp = [baseNow  += updateInterval, 0]; //data format [x, y]
                                    data.push(temp);
                                }

                                
                            };

                            data.shift(); //to remove first item of array
                            
                            while (data.length < totalPoints) {     
                                var dt = new Date();
                                var off = dt.getTimezoneOffset();
                                var dtLocal = dt.getTime() - dt.getTimezoneOffset()*60*1000;
                                var temp = [dtLocal, val]; //data format [x, y]
 
                                data.push(temp);
                            }
                    });
                    dataset = [
                        { label: "Activity Level", data: data, color: "#00FF00" }
                    ];
                    $.plot($("#realtimechart"), dataset , options);
          });
          setTimeout(updateActivity, updateInterval);
        }


    }
});
   