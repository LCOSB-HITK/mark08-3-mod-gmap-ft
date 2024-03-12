$(document).ready(function () {
    // Set your server URL
    const serverUrl = 'http://localhost:3000';

    // Get canvas context
    const canvas = document.getElementById('mapCanvas');
    const ctx = canvas.getContext('2d');

    // Initial robot position and direction
    let robot = { x: 250, y: 250, theta: 0, d_l: 0, d_r: 0 };

    const max_move = 6;
    let move = { l: 0, r:0 };
    //function getMove(){ const dist = (move.l + move.r) / 2, steer = (move.r - move.l)/(Math.abs(move.l)+Math.abs(move.r)) * Math.PI/12; }
    function getMove(){ const dist = (move.l + move.r) / 2, steer = (move.l - move.r)/(max_move*2) * Math.PI/12; return [ dist, steer ]; }
    
    let new_points = 0;
    let new_obs_win = [ 720*4, 0 ];
    function upd_obs_win(pi) { new_obs_win = [Math.min(new_obs_win[0],pi), Math.max(new_obs_win[0],pi)]; }

    let obs_points = [
        { x:0, y:0 }
    ];

    function findSortedPosition(sortedArray, newElement) {
        let low = 0;
        let high = sortedArray.length;
    
        while (low < high) {
            const mid = Math.floor((low + high) / 2);
    
            if (sortedArray[mid].y < newElement.y || (sortedArray[mid].y === newElement.y && sortedArray[mid].x < newElement.x)) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
    
        return low;
    }

    function appendNewPoint(x, y) {
        const newPoint = { x: x, y: y };
        const pi =findSortedPosition(obs_points, newPoint);
        //let pi=0;

        obs_points.splice(pi, 0, newPoint);     
        
        return pi;
    }

    function filterPoints_singlePoint(pointsList, np) {

        // Calculate the window size
        const windowSize = Math.max(2, Math.floor(pointsList.length / (720 * 4))) - 1;
    
        const pi = findSortedPosition(pointsList, np);

        // Initialize the new array for filtered points
        const filteredPoints_s = pointsList.slice(0, Math.max(0,pi-windowSize));
        const filteredPoints_e = pointsList.slice(Math.min(pointsList.length, pi+windowSize), pointsList.length);

        const window = pointsList.slice(Math.max(0,pi-windowSize), Math.min(pointsList.length, pi+windowSize));
        window.push(np);

        // calc some filtering stat
        const stdDev = calculateStandardDeviation(window);

        if (stdDev < 2) {
            // If std deviation is less than 2, take average of the window
            const avgX = window.reduce((sum, point) => sum + point.x, 0) / window.length;
            const avgY = window.reduce((sum, point) => sum + point.y, 0) / window.length;
            filteredPoints_s.push({x:avgX, y:avgY});

        } else {
            // If std deviation is more than 2, take the first element in the window
            filteredPoints_s.concat(window);
        }
    
        // Update the original points list with the filtered points
        points = filteredPoints_s.concat(filteredPoints_e);
    }
    
    function filterPoints_all(parr) {
        // Sort the points in y, then x
        //parr.sort((a, b) => Math.abs(a.y - b.y) + Math.abs(a.x - b.x));
    
        // Calculate the window size
        const windowSize = Math.max(2, Math.floor(parr.length / (720 * 2)));
        console.log('>> filterPoints_all:: windowSize', windowSize);
    
        // Initialize the new array for filtered points
        const filteredPoints = [];
    
        // Iterate over the points with a sliding window

        for (let i = 0; i < parr.length; i += windowSize) {
            const windowEnd = Math.min(i + windowSize, parr.length);
            const window = parr.slice(i, windowEnd);
    
            // Calculate standard deviation
            const stdDev = calculateStandardDeviation(window);
            console.log('>> filterPoints_all:: stddev:', stdDev);
    
            if (stdDev < windowSize*5) {
                // If std deviation is less than 2, take average of the window
                const avgX = window.reduce((sum, point) => sum + point.x, 0) / window.length;
                const avgY = window.reduce((sum, point) => sum + point.y, 0) / window.length;
                filteredPoints.push({ x: avgX, y: avgY });
            } else {
                // If std deviation is more than 2, take the first element in the window
                filteredPoints.push(...window);
            }
        }

        console.log('>> filterPoints_all:: reduced points', filteredPoints.length - parr.length);
    
        // Update the original points list with the filtered points
        parr.length = 0;
        parr.push(...filteredPoints);
    }

    // Function to calculate standard deviation
    function calculateStandardDeviation(points) {
        const n = points.length;
        if (n <= 1) return 0;
    
        const meanx = points.reduce((sum, point) => sum + point.x, 0) / n;
        const meany = points.reduce((sum, point) => sum + point.y, 0) / n;
    
        const squaredDifferencesx = points.map(point => (point.x - meanx) ** 2);
        const sumSquaredDifferencesx = squaredDifferencesx.reduce((sum, value) => sum + value, 0);
        
        const squaredDifferencesy = points.map(point => (point.y - meany) ** 2);
        const sumSquaredDifferencesy = squaredDifferencesy.reduce((sum, value) => sum + value, 0);
    
        return ( Math.sqrt(sumSquaredDifferencesx / (n - 1)) 
                + Math.sqrt(sumSquaredDifferencesy / (n - 1))
        ) / 2;
    }  

    function displayPoints() {
        // Draw each point as a red circle
        ctx.fillStyle = 'blue';
        obs_points.forEach(point => {
            ctx.beginPath();
            ctx.arc(point.x, point.y, 5, 0, 2 * Math.PI);
            ctx.fill();
        });
    }

    // Function to draw the map
    function updMap() {

        const pi = appendNewPoint( robot.x + robot.d_l * Math.cos(robot.theta + Math.PI / 2),
                        robot.y + robot.d_l * Math.sin(robot.theta + Math.PI / 2));
        updateObstacleLog(pi, obs_points[pi]);
        const pj = appendNewPoint( robot.x + robot.d_r * Math.cos(robot.theta - Math.PI / 2),
                        robot.y + robot.d_r * Math.sin(robot.theta - Math.PI / 2));
        updateObstacleLog(pj, obs_points[pj]);
        
        new_points += 2; upd_obs_win(pi); upd_obs_win(pj);

        if(false)
        if(new_points > obs_points.length*0.75) {// filter noise here

            const pre_fill = obs_points.splice(0, new_obs_win[0]);
            const post_fill= obs_points.splice(new_obs_win[1], obs_points.length);
            
            let filter_points = obs_points.splice(new_obs_win[0], new_obs_win[1]);

            filterPoints_all(filter_points)
            
            // update all
            obs_points.length = 0;
            obs_points = pre_fill.concat(filter_points, post_fill);

            new_obs_win = [ 720*4, 0 ];
            new_points = 0;
        }

        //filterPoints_all(obs_points);
        if(new_points > obs_points.length*0.25) {
            filterPoints_all(obs_points);
            new_points = 0;
        }

        // actual canvas draw
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        displayPoints();
        drawRobot(robot.x, robot.y, robot.theta);
    }

    // Function to draw the robot (arrow)
    function drawRobot(x, y, theta) {
        const arrowSize = 10;

        ctx.fillStyle = 'red';

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

        const leftSpeedElem = document.getElementById('left_speed');
        if (leftSpeedElem) {
            leftSpeedElem.innerHTML = `
                L_Speed: <strong>${move.l}</strong> 
            `;
        }
        
        const rightSpeedElem = document.getElementById('right_speed');
        if (rightSpeedElem) {
            rightSpeedElem.innerHTML = `
                R_Speed: <strong>${move.r}</strong> 
            `;
        }

        
    }

    // Function to update the robot's position based on data from the server
    function updateAll(data) {
        robot.x = data.x;
        robot.y = data.y;
        robot.theta = data.th;
        robot.d_l = data.d_l;
        robot.d_r = data.d_r;

        move.l = data.l;
        move.r = data.r;
        
        updateRobotInfoDisplay();

        updMap();
    }

    // Function to fetch data from the server periodically
    function fetchData() {
        $.ajax({
            url: serverUrl + '/GET-STATUS',
            method: 'GET',
            dataType: 'json',
            success: function (data) {
                updateAll(data);
            },
            complete: function () {
                // Fetch data again after a delay
                setTimeout(fetchData, 500); // Adjust the delay as needed
            }
        });
    }

    // Function to move the robot
    window.moveRobot = function () {
        const distance = parseFloat(prompt('Enter distance to move:')) || 0;
        const steer = 0;

        $.ajax({
            url:  serverUrl + `/MOVE?dist=${distance}&steer=${steer}`,
            method: 'GET',
            dataType: 'json',
            success: function (data) {
                updateAll(data);
            },
        });
    };

    // Function to rotate the robot
    window.rotateRobot = function (direction) {
        const distance = 0;
        const steer = direction * Math.PI *2 *15 /360 ; // Rotate by 15 degrees

        $.ajax({
            url: serverUrl + `/MOVE?dist=${distance}&steer=${steer}`,
            method: 'GET',
            dataType: 'json',
            success: function (data) {
                updateAll(data);
            },
        });
    };

    // Function to stop the robot
    window.stopRobot = function () {
        $.ajax({
            url:  serverUrl + `/STOP`,
            method: 'GET',
            dataType: 'json',
            success: function (data) {
                updateAll(data);
            },
        });
    };

    window.moveButton = function moveButton(motor, inc) {
        if(motor === 0) {//(motor+1)%2){
            move.l = Math.max(-max_move, Math.min(move.l+inc, max_move));
            console.log(">>> LEFT MOTOR")
        }
        else {
            move.r = Math.max(-max_move, Math.min(move.r+inc, max_move));
            console.log(">>> RIGHT MOTOR")
        }

        console.log('Function moveButton executed at', new Date(), 'lr ::', move.l, move.r);
        
        
        $.ajax({
            url: serverUrl + `/cMOVE?l=${move.l}&r=${move.r}`,
            method: 'GET',
            dataType: 'json',
            success: function (data) {
                updateAll(data);
            },
        });         
    }
    
    $(document).keydown(function (e) {
        switch (e.which) {
            case 38: // Up arrow key
                moveButton(1, 1);
                break;
    
            case 40: // Down arrow key
                moveButton(1, -1);
                break;
    
            case 37: // Left arrow key
                // Handle left arrow key press
                break;
    
            case 39: // Right arrow key
                // Handle right arrow key press
                break;
    
            case 87: // W key
                moveButton(0, 1);
                break;
    
            case 65: // A key
                // Handle A key press
                break;
    
            case 83: // S key
                moveButton(0, -1);
                break;
    
            case 68: // D key
                // Handle D key press
                break;
    
            // Handle other keys if needed
        }
        e.preventDefault(); // Prevent the default action (scrolling or moving the cursor)
    });
    

    // Start fetching data from the server
    fetchData();
});
