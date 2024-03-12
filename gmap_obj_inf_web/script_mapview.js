$(document).ready(function () {


    // Set your server URL
    const serverUrl = 'http://10.208.49.1'; // mesh(root) station 

    // Get canvas context
    let server_resp = "";

    
    const canvas = document.getElementById('mapCanvas');
    const ctx = canvas.getContext('2d');
    const canvasWidth = canvas.width;
    const canvasHeight = canvas.height;
    
    let curr_phy_bound = [0, 0, 0, 0];
    let curr_scale = 1;
    let curr_origin = [0, 0];

    let curr_mesh_time = 0;

    const TABLE_BREATH = 1200;
    const TABLE_LENGTH = 3000;

    const unit_color = [ 'purple', 'orange', 'black', 'white' ];


    // recalculate scale
    function updScaleWRTBound() {
        scale = Math.min(canvasWidth / (curr_phy_bound[2] - curr_phy_bound[0]), canvasHeight / (curr_phy_bound[3] - curr_phy_bound[1]));

        curr_scale = scale;
    }
    function setScaleAndTransform(s, x, y) {
        ctx.scale(s, s);
        ctx.translate(x, y);
        curr_scale = s;
        curr_origin = [x, y];
    }
    function setTransform(x, y) {
        ctx.translate(x, y);
        curr_origin = [x, y];
    }
    function setScale(s) {
        ctx.scale(s, s);
        curr_scale = s;
    }

    // GMAP OBJECTS
    let robots = {}; // key is unit_id
    function makeRobot(unit_id, x, y, theta, v, omega, rcurve, d_l, d_r, ctime) {
        return { unit_id:unit_id, x, y, theta, v, omega, rcurve, d_l, d_r, ctime };
    }

    // per unit objects
    let echo_raw_at_unit = {}; // key is unit_id
    let echo_raw_list = [
        { x:0, y:0 }
    ];

    let pl_at_unit = {}; // key is unit_id
    let pl_list = [
        { x:0, y:0, theta:0, len:0, t:0 }
    ];

    // mesh/grouped objects 
    let pl_bundles = [
        { x:0, y:0, theta:0, len:0, t:0 }
    ];
    let tables = [
        { x:0, y:0, theta:0 }
    ];

    // Function to draw the map
    function updMap() {
        // Clear the canvas
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        
        // first draw the axis
        drawAxis();

        // draw the echo_raw points
        let keys = Object.keys(robots);
        for (unit_id in keys) {
            drawPoints(echo_raw_at_unit[unit_id], unit_id);
        };

        // draw the point lines
        for (unit_id in keys) {
            drawPointLines(pl_at_unit[unit_id], unit_id);
        };

        // draw the ploint line bundles
        drawPointLines(pl_bundles, 3);

        // draw the tables
        tables.forEach(table_com => {
            drawTables(table_com);
        });
        
        // draw the robots
        for(unit_id in keys) {
            drawRobot(robots[unit_id].x, robots[unit_id].y, robots[unit_id].theta, unit_id);
        };
    }

    // all drawing functions

    function drawAxis() {
        // draw xy axis origined at curr_origin
        ctx.beginPath();

        //TODO: draw axis with grid
        
        // x- axis
        ctx.moveTo(0, curr_origin[1]);
        ctx.lineTo(canvasWidth, curr_origin[1]);
        ctx.stroke();

        // y- axis
        ctx.moveTo(curr_origin[0], 0);
        ctx.lineTo(curr_origin[0], canvasHeight);
        ctx.stroke();
    }
    /** draw points on canvas
     * can be used for echo raw points (x,y)
    */ 
    function drawPoints(points2d_list, unit_id) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.fillStyle = unit_color[unit_id];
        points2d_list.forEach(point => {
            ctx.beginPath();
            ctx.arc(point.x, point.y, 5, 0, 2 * Math.PI);
            ctx.fill();
        });
    }
    // draw point lines on canvas
    function drawPointLines(pl_list, unit_id) {
        // pl[i][0] and pl[i][1] contains the x,y position of the pl[i] point line
        // pl[i][3] contains the angle/slope of the pl[i] line (wrt to x axis)
        // for unique unit_id, draw the point(x,y) and the line with slope and of length pl[i][3] (need to scale)

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        color = unit_color[unit_id];
        pl_list.forEach(pl => {
            // calc aplha
            let color_alpha = 1-(curr_mesh_time - pl[4])/(60*3*1000); // after 3 min fade max
            color_alpha = Math.max(0.2, Math.min(1, color_alpha));
            
            // set ctx.fillStyle using color and color_aplha
            ctx.fillStyle = `rgba(${color[0]}, ${color[1]}, ${color[2]}, ${color_alpha})`;

            ctx.beginPath();

            // com point drawn
            ctx.arc(pl[0], pl[1], 5, 0, 2 * Math.PI);
            ctx.fill();

            // draw line (com is at its center)
            let x1 = pl[0] + p[3]*cos(pl[2]/1000), y1 = pl[1] + p[3]*sin(pl[2]/1000);
            let x2 = pl[0] - p[3]*cos(pl[2]/1000), y2 = pl[1] - p[3]*sin(pl[2]/1000);

            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();
        });
    }
    function drawTables(rect_com) {
        // rect_com[0:2] will have the center of the table
        // rect_com[2] will have the rotation of the table (wrt to x-axis)

        // calc the 4 corners of the table
        const dev_breath = TABLE_BREATH/2, dev_length = TABLE_LENGTH/2;
        const dev_sin = Math.sin(rect_com[2]/1000), dev_cos = Math.cos(rect_com[2]/1000);

        const x1 = rect_com[0] + dev_length * dev_cos - dev_breath * dev_sin;
        const y1 = rect_com[1] + dev_length * dev_sin + dev_breath * dev_cos;
        const x2 = rect_com[0] - dev_length * dev_cos - dev_breath * dev_sin;
        const y2 = rect_com[1] - dev_length * dev_sin + dev_breath * dev_cos;
        const x3 = rect_com[0] - dev_length * dev_cos + dev_breath * dev_sin;
        const y3 = rect_com[1] - dev_length * dev_sin - dev_breath * dev_cos;
        const x4 = rect_com[0] + dev_length * dev_cos + dev_breath * dev_sin;
        const y4 = rect_com[1] + dev_length * dev_sin - dev_breath * dev_cos;

        // draw the table
        ctx.fillStyle = 'yellow';
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.lineTo(x3, y3);
        ctx.lineTo(x4, y4);
        ctx.closePath();
        ctx.stroke();

    }
    // Function to draw the robot (arrow)
    function drawRobot(x, y, theta, unit_id) {
        const arrowSize = 10;

        ctx.fillStyle = unit_color[unit_id];

        ctx.beginPath();
        ctx.arc(x, y, 5, 0, 2 * Math.PI);
        ctx.fill();

        ctx.fillStyle = 'green';
        ctx.beginPath();
        ctx.arc(x+Math.cos(theta)*arrowSize, y+Math.sin(theta)*arrowSize, 5, 0, 2 * Math.PI);
        ctx.fill();
    }





    function updateObstacleLog(pi, newPoint) {
        const logElement = document.getElementById('obstacle-log');
        
        if (logElement) {
            // Append the new point information to the log
            logElement.value += `[${pi}:${obs_points.length}] X: ${newPoint.x.toFixed(2)}, Y: ${newPoint.y.toFixed(2)}\n`;
            // Optionally, you can also scroll to the bottom of the log
            logElement.scrollTop = logElement.scrollHeight;
        }
    }

    function updateRobotInfoDisplay() {
        const timeInfoElement = document.getElementById('robot-time');
        if (timeInfoElement) {
            timeInfoElement.innerHTML = `robot-localtime: ${robot.ctime}`;
        }

        const robotInfoElement = document.getElementById('robot-info');
        if (robotInfoElement) {
            robotInfoElement.innerHTML = `
                <strong>X:</strong> ${robot.x.toFixed(2)}, 
                <strong>Y:</strong> ${robot.y.toFixed(2)}, 
                <strong>Theta:</strong> ${robot.theta.toFixed(2)}, 
                <strong>Left Dist:</strong> ${robot.d_l}, 
                <strong>Right Dist:</strong> ${robot.d_r}
            `;
        }

        const respInfoElement = document.getElementById('resp-info');
        if (respInfoElement) {
            respInfoElement.innerHTML = `server-resp:${server_resp}`;
        }

        const leftSpeedElem = document.getElementById('left_speed');
        if (leftSpeedElem) {
            leftSpeedElem.innerHTML = `
                L_Speed: <strong>${cmotor.l}</strong> 
            `;
        }
        
        const rightSpeedElem = document.getElementById('right_speed');
        if (rightSpeedElem) {
            rightSpeedElem.innerHTML = `
                R_Speed: <strong>${cmotor.r}</strong> 
            `;
        }

        const wsadSpeedElem = document.getElementById('wsad_speed');
        if (wsadSpeedElem) {
            wsadSpeedElem.innerHTML = `
                pow: <strong>${move.p}</strong> steer: <strong>${move.s}</strong> 
            `;
        }
        
    }

    function showResp(resp) {
        server_resp = String(resp);
        
        const respInfoElement = document.getElementById('resp-info');
        if (respInfoElement) {
            respInfoElement.innerHTML = `server-resp:${server_resp}`;
        }
    }

    function parsePlainTextResponse(data) {
        var values = data.split(' ');
        
        robot.ctime = parseInt(values[0]);
        robot.x = parseInt(values[1]);
        robot.y = parseInt(values[2]);
        robot.theta = parseInt(values[3]);
        robot.v = parseInt(values[4]);
        robot.omega = parseInt(values[5]);
        robot.rcurve = parseInt(values[6]);
        cmotor.l = parseInt(values[7]);
        cmotor.r = parseInt(values[8]);
        robot.d_l = parseInt(values[9]);
        robot.d_r = parseInt(values[10]);

        updSteerMove();
    }

    // Function to update the robot's position based on data from the server
    function updateAll(data) {
        // robot.x = data.x;
        // robot.y = data.y;
        // robot.theta = data.th;
        // robot.d_l = data.d_l;
        // robot.d_r = data.d_r;

        // cmotor.l = data.l;
        // cmotor.r = data.r;
        console.log(">> updateAll")
        parsePlainTextResponse(data);

        updateRobotInfoDisplay();

        updMap();
    }

    // Function to fetch data from the server periodically
    function fetchData() {
        $.ajax({
            url: serverUrl + '/map_inf',
            method: 'GET',
            dataType: 'aplication/json',
            success: function (data) {
                console.log(">> fetch success:");
                console.log(data);
                updateAll(data);
            },
            error: function (jqXHR, textStatus, errorThrown) {
                console.error(">> fetch error:", textStatus, errorThrown);
            },
            complete: function () {
                // Fetch data again after a delay
                console.log(">> fetch complete");
                setTimeout(fetchData, 1000); // Adjust the delay as needed
            }
        });
    }

    // function capCam() {
    //     $.ajax({
    //         url: camServerUrl + `/SDCapture?flash=${flash}&fpm=0`,
    //         method: 'GET',
    //         dataType: 'text',
    //         success: function (data) {
    //             showResp(data);
    //         },
    //         error: function (jqXHR, textStatus, errorThrown) {
    //             console.error(">> fetch error:", textStatus, errorThrown);
    //         }
    //     });
    // }


    // Function to move the robot
    window.moveRobot = function () {
        const distance = parseFloat(prompt('Enter distance to move:')) || 0;
        const steer = 0;

        //TODO: srever side setGPos uri
        // $.ajax({
        //     url: serverUrl + '/moveRover',
        //     method: 'POST',
        //     contentType: 'text', 
        //     data: `pow=${distance} steer=${steer}`,
        //     success: function (data) {
        //         showResp(data);
        //     },
        // });
    };

    // Function to rotate the robot
    window.rotateRobot = function (direction) {
        const distance = 0;
        const steer = direction * 2 ; // Rotate by 15 degrees

        $.ajax({
            url: serverUrl + "/moveRover",
            method: 'POST',
            contentType: 'text',
            data: `pow=${distance} steer=${steer}`,
            success: function (data) {
                showResp(data);
            },
        });
    };

    // Function to stop the robot
    window.stopRobot = function () {
        $.ajax({
            url:  serverUrl + `/stopRover`,
            method: 'GET',
            dataType: 'text',
            success: function (data) {
                showResp(data);
            },
        });
    };

    window.moveButton = function moveButton(motor, inc) {
        if(motor === 0)
                cmotor.l = Math.max(-max_motor, Math.min(cmotor.l+inc, max_motor));
        else    cmotor.r = Math.max(-max_motor, Math.min(cmotor.r+inc, max_motor));
        
        $.ajax({
            url: serverUrl + "/ctrlMotor",
            method: 'POST',
            dataType: 'text',
            data:`l_m=${cmotor.l} r_m=${cmotor.r}`,
            success: function (data) {
                showResp(data);
            },
        });         
    }

    window.wsadMove = function wsadMove(d, s) {
        move.p = Math.max(-max_motor, Math.min(move.p+d, max_motor));
        move.s = Math.max(-max_steer, Math.min(move.s+s, max_steer));

        $.ajax({
            url: serverUrl + "/moveRover",
            method: 'POST',
            dataType: 'text',
            data:`pow=${move.p} steer=${move.s}`,
            success: function (data) {
                showResp(data);
            },
        });         
    }
    
    $(document).keydown(function (e) {
        switch (e.which) {
            case 38: // Up arrow key
                //wsadMove(1, 1);
                break;
    
            case 40: // Down arrow key
                //moveButton(1, -1);
                break;
    
            case 37: // Left arrow key
                // Handle left arrow key press
                break;
    
            case 39: // Right arrow key
                // Handle right arrow key press
                break;
    
            case 87: // W key
                wsadMove(1, 0);
                break;
    
            case 65: // A key
                wsadMove(0, 1);
                break;
    
            case 83: // S key
                wsadMove(-1,0);
                break;
    
            case 68: // D key
                wsadMove(0,-1);
                break;
            
            // Handle other keys if needed
        }
        e.preventDefault(); // Prevent the default action (scrolling or moving the cursor)
    });
    

    // Start fetching data from the server
    fetchData();
});
