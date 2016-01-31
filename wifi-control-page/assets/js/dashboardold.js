var data = []
var updateInterval = 3000;

    
    if($("#realtimechart").length)
    {
        var options = {
            series: { shadowSize: 1 },
            lines: { fill: true, fillColor: { colors: [ { opacity: 1 }, { opacity: 0.1 } ] }},
            yaxis: { min: 0, max: 100 },
            xaxis: { show: false },
            colors: ["rgba(65,139,202,0.5)"],
            grid: { tickColor: "#dddddd",
                    borderWidth: 0 
            },
        };

        function update() { 
          $.getJSON( "./api/v1/lastlevel", function( datatemp ) {
                    $.each( datatemp, function(key, val) {
                        if (data.length == 0)
                        {
                            for (var i = 0; i <= 300; i++) {
                                data.push(val);
                            };
                        }
                        else
                        {
                            data.push(val);
                        }
                    });
                    if (data.length > 0) data.slice(1);
                    var res = [];
                    for (var i = 0; i < data.length; ++i)
                    res.push([i, data[i]])
                    $.plot($("#realtimechart"), [ res ], options);
          });
          setTimeout(update, updateInterval);
        }

        update();
    }

   