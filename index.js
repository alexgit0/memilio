// function init_graph() {


// }

var data_read = [];

var xScale, yScale, xScaleMini, yScaleMini;

var selectLine;
var selectLineTooltip = [];
var selectLineCircle = [];

// var div_locations = d3.select("#locations").style("overflow", "hidden");

// div_locations.append('div')
//     .style("width", "100%")
//     .style("height", "26px")
//     .style("padding", "7px")
//     .style("padding-left", "15px")
//     .style("background-color", "#73B0FF")
//     .html("Orte")
//     .style("font-weight", "bold");


// div_locations.append("svg")
//     .attr("width", 200)
//     .attr("height", 200)
//     .append("button")
//     .html("asdasd")
//     .attr("x", 100)
//     .attr("y", 100);


var svg_locations = d3.select("#locations");

svg_locations.append('rect')
    .attr("width", "100%")
    .attr("height", "40")
    .style("fill", "#73B0FF");

svg_locations.append('text')
    .html("Orte")
    .attr("x", "20")
    .attr("y", "25")
    .attr("font-weight", "bold");


var loc = ["D&uuml;sseldorf", "K&ouml;ln", "Aachen", "Bonn"];
var loc_length = [10, 4, 6, 4]
var loc_ref = ["data_01", "data_02", "data_03", "data_04"];
var loc_pos = [
    [150, 175],
    [200, 350],
    [75, 475],
    [225, 525]
];

var data_chosen;

for (var i = 0; i < loc.length; i++) {
    svg_locations.append("circle")
        .attr("class", "button")
        .attr("r", 50)
        .attr("cx", loc_pos[i][0])
        .attr("cy", loc_pos[i][1])
        .attr("fill", "#cdcdcd")
        .attr("alt", loc_ref[i])
        .on("click", onButtonMouseClick) //Add listener for the click event
        .on("mouseover", onButtonMouseOver) //Add listener for the mouseover event 
        .on("mouseout", onButtonMouseOut); //Add listener for the mouseout event 

    svg_locations.append("text")
        .attr("x", loc_pos[i][0] - 4 * loc_length[i])
        .attr("y", loc_pos[i][1] + 5)
        .attr("alt", loc_ref[i])
        .on("click", onButtonMouseClick) //Add listener for the click event
        .on("mouseover", onTextMouseOver)
        .html(loc[i])
        .attr("font-weight", 600);
}

// alert(loc[1].length)


// var div_actions = d3.select("#actions").style("overflow", "hidden");

// div_actions.append('div')
//     .style("width", "100%")
//     .style("height", "26px")
//     .style("padding", "7px")
//     .style("padding-left", "15px")
//     .style("background-color", "#73B0FF")
//     .html("Maßnahmen")
//     .style("font-weight", "bold");

// var div_parameters = d3.select("#parameters").style("overflow", "hidden");

// div_parameters.append('div')
//     .style("width", "100%")
//     .style("height", "26px")
//     .style("padding", "7px")
//     .style("padding-left", "15px")
//     .style("background-color", "#73B0FF")
//     .html("Parameter")
//     .style("font-weight", "bold");




var svg_graphs = d3.select("#graphs"),
    margin_top = 300,
    margin_bottom = 50,
    margin_vertical = margin_top + margin_bottom,
    margin_horizontal = 200,
    width = svg_graphs.attr("width") - margin_horizontal,
    height = svg_graphs.attr("height") - margin_vertical,
    heightMini = margin_top / 4;

var coeffs_critical = [0.2, 0.1, 0.05];
var colors = ["#73B0FF", "orange", "red", "black"];

var formatTime = d3.timeFormat("%d.%m.%Y"); // this is actually a function... formatTime(d)

var overlay_mini = [];

var begin_interval_i = 0,
    end_interval_i = 60,
    begin_interval_x = 0,
    end_interval_x = 60,
    mouse_down_on_mini = false;

var g = [];

svg_graphs.append('rect')
    .attr("width", "100%")
    .attr("height", "40")
    .style("fill", "#73B0FF");

svg_graphs.append('text')
    .html("Verlauf")
    .attr("x", "20")
    .attr("y", "25")
    .attr("font-weight", "bold");


function read_and_visualize(filename, log_scale) {

    logscale = log_scale;
    max_fact = 1;

    svg_graphs.selectAll("*").remove();
    if (data_read.length > 0) { // how to delete data?
        delete(data_read[0]);
        delete(data_read[1]);
        data_read = [];
    }

    svg_graphs.append('rect')
        .attr("width", "100%")
        .attr("height", "40")
        .style("fill", "#73B0FF");

    svg_graphs.append('text')
        .html("Verlauf")
        .attr("x", "20")
        .attr("y", "25")
        .attr("font-weight", "bold");


    // var xScale = d3.scaleBand().range([0, width]).padding(0);
    xScale = d3.scaleTime().range([0, width]);
    if (logscale) {
        yScale = d3.scaleLog().range([height, 0]);
        max_fact = 5;
    } else {
        yScale = d3.scaleLinear().range([height, 0]);
    }

    // var xScaleMini = d3.scaleBand().range([0, width]).padding(0);
    xScaleMini = d3.scaleLinear().range([0, width]);
    if (logscale) {
        yScaleMini = d3.scaleLog().range([heightMini, 0]);
    } else {
        yScaleMini = d3.scaleLinear().range([heightMini, 0]); // inverted.....
    }

    svg_graphs.append('g')
        .append("text")
        .attr("transform", "rotate(-90)")
        .attr("class", "ylabel")
        .attr("text-anchor", "middle")
        .attr("y", 0.25 * margin_horizontal)
        .attr("x", -margin_top - 0.5 * height)
        .text("Fälle");

    svg_graphs.append('g')
        .append("text")
        .attr("class", "xlabel")
        .attr("text-anchor", "middle")
        .attr("x", 0.5 * width + 0.5 * margin_horizontal)
        .attr("y", margin_top + 0.75 * margin_bottom + height)
        .text("Datum");

    g[0] = svg_graphs.append("rect")
        .attr("width", width)
        .attr("height", height)
        .attr('fill', 'none')
        .attr("transform", "translate(" + 0.5 * margin_horizontal + "," + margin_top + ")")
        .attr("pointer-events", "all")
        // .on("mouseover", onMouseOver) //Add listener for the mouseover event
        .on("mousemove", onMouseMove) //Add listener for the mouseout event 
        // .on("mouseout", onMouseOut); //Add listener for the mouseout event 


    g[1] = svg_graphs.append("rect")
        .attr("width", width)
        .attr("height", heightMini)
        .attr('fill', 'none')
        .attr("transform", "translate(" + 0.5 * margin_horizontal + "," + (margin_top - 1.5 * heightMini) + ")")
        .attr("pointer-events", "all")
        .on("mousemove", onMouseMoveMini) //Add listener for the mouseover event
        .on("mousedown", onMouseDownMini) //Add listener for the down event 
        // .on("mouseleave", onMouseLeaveMini) //Add listener for the down event 
        .on("mouseup", onMouseReleaseMini) //Add listener for the release event 
        // .on("mouseout", onMouseOutMini); //Add listener for the mouseout event 


    g[0] = svg_graphs.append("g")
        .attr("id", "main_graph")
        // .attr("width", width) // ist wohl nicht noetig.. mouse events werden auch so durch das rect oben gefangen und nicht durch das g welches ein dimensionsloser Container ist
        // .attr("height", height)
        // .attr('fill', 'none')
        .attr("transform", "translate(" + 0.5 * margin_horizontal + "," + margin_top + ")");


    g[1] = svg_graphs.append("g")
        .attr("id", "mini_graph")
        .attr("transform", "translate(" + 0.5 * margin_horizontal + "," + (margin_top - 1.5 * heightMini) + ")");


    overlay_mini = g[1].append("rect")
        .attr("id", "gray_overlay")
        .style("opacity", 0.2)
        .on("mousemove", onMouseMoveMini) //Add listener for the mouseover event
        .on("mousedown", onMouseDownMini) //Add listener for the down event 
        .on("mouseup", onMouseReleaseMini) //Add listener for the release event 
        .attr("x", begin_interval_x)
        .attr("width", end_interval_x - begin_interval_x)
        .attr("height", heightMini); // linker ausgegrauter Bereich


    d3.csv(filename, function(d) {
            return { day: d3.timeParse("%d.%m.%Y")(d.day), cases: [parseInt(d.cases), parseInt(coeffs_critical[0] * d.cases), parseInt(coeffs_critical[1] * d.cases), parseInt(coeffs_critical[2] * d.cases)] }
        },

        function(error, data) {
            if (error) {
                throw error;
            }

            data_read.push(data);

            if (begin_interval_i == 0 && end_interval_i == 60) {
                filteredData = data.filter(function(d) { return d.day < data[end_interval_i].day });
                data_read.push(filteredData);
            } else {
                data_read.push(data.slice(begin_interval_i, end_interval_i));
            }

            console.log(data_read[1]);


            xScale.domain(d3.extent(data_read[1], function(d) { return d.day; }));
            yScale.domain([0, max_fact * d3.max(data_read[1], function(d) { return d.cases[0]; })]);


            g[0].append("g")
                .attr("id", "xaxis")
                .attr("transform", "translate(0," + height + ")")
                .call(d3.axisBottom(xScale).tickFormat(d3.timeFormat("%d.%m.%Y")).ticks(6));
            // .tickFormat(d3.timeFormat("%Y-%m-%d")))
            // .selectAll("text")
            // .style("text-anchor", "end")
            // .attr("dx", "-.8em")
            // .attr("dy", ".15em")
            // .attr("transform", "rotate(-65)");

            g[0].append("g")
                .attr("id", "yaxis")
                .call(d3.axisLeft(yScale).ticks(4));

            if (begin_interval_i == 0 && end_interval_i == 60) {
                for (var i = 0; i < data_read[0][0].cases.length; i++) {

                    g[0].append("path")
                        .datum(filteredData)
                        .attr("fill", "none")
                        .attr("stroke", colors[i])
                        .attr("stroke-width", 1.5)
                        .attr("d", d3.line()
                            .x(function(d) { return xScale(d.day) })
                            .y(function(d) { return yScale(d.cases[i]) })
                        )

                }
            } else {
                update_graph();
            }

            // define bars in graphic
            // g.selectAll(".bar_cat1")
            //     .data(filteredData)
            //     .enter();
            // .append("rect")
            // .attr("class", "bar_cat1")
            // .on("mouseover", onMouseOver) //Add listener for the mouseover event
            // .on("mouseout", onMouseOut) //Add listener for the mouseout event       
            // .attr("x", function(d) { return xScale(d.day); })
            // .attr("y", function(d) { return yScale(d.cases); })
            // .attr("width", xScale.bandwidth())
            // .attr("height", function(d) { return height - yScale(d.cases); });

            // mini graph
            xScaleMini.domain(d3.extent(data_read[0].map(function(d) { return d.day; })));
            yScaleMini.domain([1, max_fact * d3.max(data_read[0], function(d) { return d.cases[0]; })]);

            overlay_mini.attr("width", end_interval_x - begin_interval_x);

            g[1].append("g")
                .call(d3.axisLeft(yScaleMini).tickFormat(function(d) {
                    return d;
                }).ticks(4));
            // .attr("transform", "translate(0," + (-margin + 10) + ")");


            g[1].append("g")
                .attr("transform", "translate(0," + heightMini + ")")
                .call(d3.axisBottom(xScaleMini)
                    .ticks(6)
                    .tickFormat(d3.timeFormat("%d.%m.%Y")));

            // Add the line
            for (var i = 0; i < data_read[0][0].cases.length; i++) {
                g[1].append("path")
                    .datum(data_read[0])
                    .attr("fill", "none")
                    .attr("stroke", colors[i])
                    .attr("stroke-width", 1.5)
                    .attr("d", d3.line()
                        .x(function(d) { return xScaleMini(d.day) })
                        .y(function(d) { return yScaleMini(d.cases[i]) })
                    )
            }


            // actions
            g[2] = svg_graphs.append("g")
                .attr("transform", "translate(" + 0.5 * margin_horizontal + "," + 10 + ")");

            g[2].append("g")
                .attr("transform", "translate(0," + 2 * heightMini + ")")
                .call(d3.axisBottom(xScaleMini)
                    .ticks(6)
                    .tickFormat(d3.timeFormat("%d.%m.%Y")));

            // Add the line
            g[2].append("rect")
                .attr("class", "bar_cat1")
                .attr("x", function(d) { return 10; })
                .attr("y", function(d) { return 2 * heightMini - 10; })
                .attr("width", 10)
                .attr("height", function(d) { return 10; });


            // cursor on main graph
            selectLine = svg_graphs.append("rect")
                .style("opacity", 0)
                .attr("width", 2)
                .attr("height", function(d) { return height; })
                .attr("x", 0.75 * margin_horizontal)
                .attr("y", margin_top);


            for (var i = 0; i < data_read[0][0].cases.length; i++) {
                selectLineCircle[i] = svg_graphs.append('g')
                    .append('circle')
                    .style("opacity", 0)
                    .attr('r', 4)
                    .style("fill", colors[i])
                    .attr("cx", 0.75 * margin_horizontal)
                    .attr("cy", margin_top);
                // .append('circle')
                // .style("opacity", 0)
                // .attr('r', 4)
                // .attr('color', colors[i])
                // .attr("text-anchor", "left")
                // .attr("alignment-baseline", "middle")
                // .attr("x", 0.75 * margin)
                // .attr("y", height - 0.5 * margin);
            }

            for (var i = 0; i < data_read[0][0].cases.length + 1; i++) {
                selectLineTooltip[i] = svg_graphs.append('g')
                    .append('text')
                    .style("opacity", 0)
                    .attr("text-anchor", "left")
                    .attr("alignment-baseline", "middle")
                    .attr("x", 0.5 * margin_horizontal)
                    .attr("y", margin_top);
            }

            // selectLine.append("g")
            //     .html("asds");
            // selectLineTooltip = d3.select("svg")
            // .append("rect")
            // .style("opacity", 0)
            // .attr("width", 0)
            // .attr("height", function(d) { return height; })
            // .style("left", 0.75 * margin + 10)
            // .style("top", height - 0.5 * margin + 10);

            console.log(data);

            // how to delete data?
            // delete(data);
            // delete(filteredData);
        });
}




function update_graph(log_scale) {

    g[0].selectAll("path").remove();
    g[0].select("#xaxis").remove();
    g[0].select("#yaxis").remove();

    xScale.domain(d3.extent(data_read[1], function(d) { return d.day; }));
    yScale.domain([0, max_fact * d3.max(data_read[1], function(d) { return d.cases[0]; })]);


    g[0].append("g")
        .attr("id", "xaxis")
        .attr("transform", "translate(0," + height + ")")
        .call(d3.axisBottom(xScale).tickFormat(d3.timeFormat("%d.%m.%Y")).ticks(6));
    // .tickFormat(d3.timeFormat("%Y-%m-%d")))
    // .selectAll("text")
    // .style("text-anchor", "end")
    // .attr("dx", "-.8em")
    // .attr("dy", ".15em")
    // .attr("transform", "rotate(-65)");

    g[0].append("g")
        .attr("id", "yaxis")
        .call(d3.axisLeft(yScale).ticks(4));

    for (var i = 0; i < data_read[0][0].cases.length; i++) {

        g[0].append("path")
            .datum(data_read[1])
            .attr("fill", "none")
            .attr("stroke", colors[i])
            .attr("stroke-width", 1.5)
            .attr("d", d3.line()
                .x(function(d) { return xScale(d.day) })
                .y(function(d) { return yScale(d.cases[i]) })
            )

    }
}


function subsample_data() {

    delete(data_read[1]);

    data_read[1] = data_read[0].slice(begin_interval_i, end_interval_i);

}


// function onMouseOver() {

// }

function onMouseMove() {
    // get coordinates
    var x0_pos = d3.mouse(this)[0];
    var bisectDate = d3.bisector(function(d) { return d.day; }).left;
    var i = bisectDate(data_read[1], xScale.invert(x0_pos))
    var selectedData = data_read[1][i - 1]
    x0_pos = xScale.invert(x0_pos + 0.5 * margin_horizontal);

    // set bar to move
    selectLine
        .style("opacity", 0.25)
        .attr("x", xScale(x0_pos));

    // set date to move with bar
    selectLineTooltip[0]
        .style("opacity", 1)
        .attr("text-anchor", "left")
        .attr("alignment-baseline", "middle")
        .attr("x", (xScale(x0_pos) + 10))
        .attr("y", margin_top)
        .html(formatTime(selectedData.day));

    // set cases numbers and circles to move with bar
    for (var i = 0; i < data_read[1][0].cases.length; i++) {

        selectLineTooltip[i + 1]
            .style("opacity", 1)
            .attr("text-anchor", "left")
            .attr("alignment-baseline", "middle")
            .attr("x", (xScale(x0_pos) + 10))
            .attr("y", margin_top + yScale(selectedData.cases[i]))
            .html(selectedData.cases[i]);

        selectLineCircle[i]
            .style("opacity", 1)
            .attr("cx", xScale(x0_pos))
            .attr("cy", margin_top + yScale(selectedData.cases[i]))
            .html(selectedData.cases[i]);

    }
}

function onMouseDownMini() {
    // get coordinates
    var x0_pos = d3.mouse(this)[0];
    var bisectDate = d3.bisector(function(d) { return d.day; }).left;
    var i = bisectDate(data_read[0], xScaleMini.invert(x0_pos));

    begin_interval_i = i - 1;
    begin_interval_x = x0_pos;

    overlay_mini.attr("x", x0_pos);

    mouse_down_on_mini = true;
}

function onMouseMoveMini() {

    if (mouse_down_on_mini) {

        end_interval_x = d3.mouse(this)[0];

        if (end_interval_x - begin_interval_x > 0) {
            overlay_mini.attr("width", end_interval_x - begin_interval_x);
        }
    }

}

function onMouseReleaseMini() {

    end_interval_x = d3.mouse(this)[0];

    if (end_interval_x - begin_interval_x > 0) {
        overlay_mini.attr("width", end_interval_x - begin_interval_x);

        var bisectDate = d3.bisector(function(d) { return d.day; }).left;
        var i = bisectDate(data_read[0], xScaleMini.invert(end_interval_x));

        end_interval_i = i - 1;

        subsample_data();

        update_graph(false);
    }

    mouse_down_on_mini = false;
}

// function onMouseLeaveMini() {

//     end_interval_x = d3.mouse(this)[0];

//     if (end_interval_x - begin_interval_x > 0) {
//         overlay_mini.attr("width", end_interval_x - begin_interval_x);

//         var bisectDate = d3.bisector(function(d) { return d.day; }).left;
//         var i = bisectDate(data_read[0], xScaleMini.invert(end_interval_x));

//         end_interval_i = i - 1;
//     }

//     mouse_down_on_mini = false;
// }


function onButtonMouseClick() {

    d3.select(".button_active").attr("class", "button")

    var activeElem = d3.select(this);

    read_and_visualize(activeElem.attr("alt") + ".csv", false);

    data_chosen = d3.select(this).attr("alt");
    if (activeElem.node().tagName == "circle") {
        activeElem.attr("class", "button_active")
    }


    d3.selectAll(".button").transition(200)
        .attr('fill', '#cdcdcd');

}

function onButtonMouseOver() {
    d3.select(this)
        .transition(400)
        .attr('fill', '#73B0FF');

    d3.select(this).style("cursor", "pointer");
}

function onTextMouseOver() {
    d3.select(this).style("cursor", "pointer");
}


function onButtonMouseOut() {

    if (data_chosen != d3.select(this).attr("alt")) {

        d3.select(this)
            .transition(200)
            .attr('fill', '#cdcdcd');
    }

}

// //mouseout event handler function
// function onMouseOut(d) {
//     // use the text label class to remove label on mouseout
//     d3.select(this).attr('class', 'bar_cat1');

//     d3.selectAll('.val')
//         .remove()
// }