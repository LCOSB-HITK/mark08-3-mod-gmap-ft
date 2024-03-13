$(document).ready(function () {

    // Set your server URL
    const serverUrl = 'http://10.208.49.1'; // mesh(root) station 

    // Get canvas context
    let server_resp = "";


    // Get canvas context
    const canvas = document.getElementById('mapCanvas');
    const ctx = canvas.getContext('2d');
    const canvasWidth = canvas.width;
    const canvasHeight = canvas.height;

    // Initial robot position and direction
    let robot = { unit_id:0, x: 0, y: 0, theta: 0, v:0, l:0, r:0, omega:0, rcurve:0, d_l: 0, d_r: 0, ctime:0 };

    const max_motor = 10;
    const max_steer = 10;

    let cmotor = { l: 0, r: 0 };
    let move =   { p: 0, s: 0 };

    //function getMove(){ const pow = (cmotor.l + cmotor.r) / 2, steer = (cmotor.r - cmotor.l)/(Math.abs(cmotor.l)+Math.abs(cmotor.r)) * Math.PI/12; }
    function updSteerMove(){ 
        const pow =   (cmotor.l + cmotor.r) / 2;
        const steer = (cmotor.r - cmotor.l) / 2 *Math.sign(pow);
        move.p = pow;
        move.s = steer;
        return [ pow, steer ]; 
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

    function updateRobotArrowDisplay() {

        // Draw the robot
        const arrowSize = CANVAS_MAX_SPEED*robot.v;

        let x = canvasWidth/2 +Math.cos(robot.theta)*arrowSize;
        let y = canvasHeight/2+Math.sin(robot.theta)*arrowSize;

        // draw line
        ctx.fillStyle = 'red';
        ctx.beginPath();
        ctx.moveTo(canvasWidth/2, canvasHeight/2);
        ctx.lineTo(x, y);
        ctx.stroke();

        // draw arrow/triangle
        ctx.fillStyle = 'green';
        ctx.beginPath();
        ctx.arc(x, y, 5, 0, 2 * Math.PI);
        ctx.fill();

        // draw Echo Marks

        // for d_r
        ctx.fillStyle = robot.d_r >= 5000 ? 'blue' : 'yellow';
        ctx.beginPath();
        ctx.moveTo(canvasWidth/2, canvasHeight/2);
        ctx.lineTo(canvasWidth/2 + Math.cos(robot.theta - Math.PI/2) * robot.d_r, canvasHeight/2 + Math.sin(robot.theta - Math.PI/2) * robot.d_r);
        ctx.stroke();

        // for d_l
        ctx.fillStyle = robot.d_l >= 5000 ? 'blue' : 'yellow';
        ctx.beginPath();
        ctx.moveTo(canvasWidth/2, canvasHeight/2);
        ctx.lineTo(canvasWidth/2 + Math.cos(robot.theta + Math.PI/2) * robot.d_l, canvasHeight/2 + Math.sin(robot.theta + Math.PI/2) * robot.d_l);
        ctx.stroke();
        
    }

    function showResp(resp) {
        server_resp = String(resp);
        
        const respInfoElement = document.getElementById('resp-info');
        if (respInfoElement) {
            respInfoElement.innerHTML = `server-resp:${server_resp}`;
        }
    }

    function parsePlainTextRobotStat(data) {
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
    function updateDisp() {
        console.log(">> updateDisp")

        updateRobotInfoDisplay();

        updateRobotArrowDisplay();
    }

    // Function to fetch data from the server periodically
    function fetchRobotStat() {
        $.ajax({
            url: serverUrl + `/robot_stat?nid=${robot.unit_id}`,
            method: 'GET',
            dataType: 'text',
            success: function (data) {
                console.log(">> fetch success:");
                console.log(data);
                updateRobotStat(data);
                updateDisp();
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
    function updateRobotStat(data) {
        // Check if the data contains the 'robots' array
        if (data.hasOwnProperty('robots')) {
            // Iterate over each robot in the 'robots' array
            data.robots.forEach(robotData => {
                // Extract attributes from the robot data
                let { unit_id, x, y, theta, v, omega, rcurve, l, r, d_l, d_r, ctime } = robotData;

                if (unit_id === robot.unit_id) {

                    // Update the robot's position
                    robot.x = x;
                    robot.y = y;
                    robot.theta = theta;
                    robot.v = v;
                    robot.omega = omega;
                    robot.rcurve = rcurve;
                    robot.l = l;
                    robot.r = r;
                    robot.d_l = d_l;
                    robot.d_r = d_r;
                    robot.ctime = ctime;

                    //IMP:
                    updSteerMove();
                }
                else if (robot.unit_id === 0) {
                    // Create a robot object
                    robot = {unit_id, x, y, theta, v, omega, rcurve, l, r, d_l, d_r, ctime};
                }
            });
        }
    }

    // Functions for robot control
    function reqMoveRover(p, s) {
        var command = { type: "moveRover", p: p, s: s };

        $.ajax({
            url: serverUrl + `/node-fwd/?nid=${robot.unit_id}&type=moveRover`,
            method: 'POST',
            dataType: 'text',
            data: JSON.stringify(command),
            success: function (data) {
                showResp(data);
            },
        });
    }
    window.stopRobot = function () {

        var command = { type: "stopRover" };

        $.ajax({
            url:  serverUrl + `/node-fwd/?nid=${robot.unit_id}&type=stopRover`,
            method: 'POST',
            dataType: 'text',
            data: JSON.stringify(command),
            success: function (data) {
                showResp(data);
            },
        });
    }
    window.setMotor = function setMotor(motor, inc) {
        if(motor === 0)
                cmotor.l = Math.max(-max_motor, Math.min(cmotor.l+inc, max_motor));
        else    cmotor.r = Math.max(-max_motor, Math.min(cmotor.r+inc, max_motor));
        
        var command = { type: "ctrlMotor", r: cmotor.r, l: cmotor.l };

        // Send command JSON object as POST data
        $.ajax({
            url: serverUrl + `/node-fwd/?nid=${robot.unit_id}&type=ctrlMotor`,
            method: 'POST',
            dataType: 'text',
            data: JSON.stringify(command), // Convert JSON object to string
            success: function(data) {
                showResp(data);
            },
        });         
    }
    window.rotateRobot = function (direction) {
        const distance = 0;
        const steer = direction * 2 ; // Rotate by 15 degrees

        reqMoveRover(distance, steer);
    }
    window.wsadMove = function wsadMove(d, s) {
        move.p = Math.max(-max_motor, Math.min(move.p+d, max_motor));
        move.s = Math.max(-max_steer, Math.min(move.s+s, max_steer));

        reqMoveRover(move.p, move.s);
    }

    
    $(document).keydown(function (e) {
        switch (e.which) {
            case 38: // Up arrow key
                //wsadMove(1, 1);
                break;
    
            case 40: // Down arrow key
                //setMotor(1, -1);
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

            case 32: // Space key
                stopRobot();
                break;
            
            case 81: // Q key
                rotateRobot(1);
                break;
            
            case 69: // E key
                rotateRobot(-1);
                break;
            // Handle other keys if needed
        }
        e.preventDefault(); // Prevent the default action (scrolling or moving the cursor)
    });
    

    // Start fetching data from the server
    
});
