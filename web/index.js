var values_x1 = [];
var values_x2 = [];
var columns_colors = []; 
var colors = ["brown", "red", "orange", "purple", "cyan"]
var current_color = 0;
var values_y1 = [];
var values_y2 = [];
var ch1;
var ch2;
var allElements = [];
var correctExecutions = [];
var sumTemp = 0;
var sumHum = 0;

function displayDataOnChart(input){
    input = JSON.parse(input);
    for(let i=0;i < input.length; i++){
        if(input[i]["Temperature"] > 30 || input[i]["Humidity"] > 40)
            correctExecutions.push(input[i]);
        sumTemp += parseInt(input[i]["Temperature"]);
        sumHum += parseInt(input[i]["Humidity"]);
        allElements.push(input[i]);
        current_date=new Date();
        hour = current_date.getHours(); 
        minute = current_date.getMinutes();
        label = "Time: " + hour.toString()+":" + minute.toString();
        columns_colors.push(colors[current_color%5]);
        current_color++;
        ch1.data.labels.push(label);
        ch1.data.datasets[0].data.push(input[i]["Temperature"]);
        ch1.update();
        ch2.data.labels.push(label);
        ch2.data.datasets[0].data.push(input[i]["Humidity"]); 
        ch2.update();
    }
    document.getElementById("correctExecutions").innerHTML = correctExecutions.length;
    document.getElementById("counter").innerHTML = allElements.length;
    document.getElementById("avgTemp").innerHTML = sumTemp/allElements.length;
    document.getElementById("avgHum").innerHTML = sumHum/allElements.length;


}
function callAPI(){
    var myHeaders = new Headers();
    var requestOptions = {
        method: 'GET',
        headers: myHeaders,
    };
    fetch("https://2rgqx1lmwc.execute-api.eu-west-1.amazonaws.com/dev", requestOptions)
    .then(response => response.text())
    .then(result => displayDataOnChart(JSON.parse(result).body))  
}
function createPlot(name, val_y, val_x){
    ch = new Chart(name, {
        type: "bar",
        data: {
            labels: val_x,
            datasets: [{
            backgroundColor: columns_colors,
            data: val_y
            }]
        },
        options: {
            legend: {display: false},
            title: {
            display: true,
            },
            scales: {
                yAxes: [{
                  ticks: {
                    beginAtZero: true
                  }
                }],
              }
        }
        });
    return ch;
}
function init(){   
    ch1=createPlot("Temperature", values_y1, values_x1);
    ch2=createPlot("Humidity", values_y2, values_x2);
    callAPI();
}
  