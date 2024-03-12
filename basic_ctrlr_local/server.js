const express = require('express');
const cors = require('cors');
const app = express();
const port = 3000;

const fs = require('fs');
const path = require('path');
const Jimp = require('jimp');

const max_move = 6;

// all routes
app.use(cors());

// Initial robot position and map
let move = { l:0, r:0 };
function getMove(){ const dist = (move.l + move.r) / 2, steer = (move.l - move.r)/(max_move*2) * Math.PI/12; return [ dist, steer ]; }

let robot = { x: 250, y: 250, theta: 0, d_l:0, d_r:0 };
let obstacleMap = null;

// Load the bitmap image as the map
function loadMap() {
    const imagePath = path.join(__dirname, 'map1.bmp');

    return Jimp.read(imagePath)
        .then(image => {
            obstacleMap = image;
            console.log('Map loaded successfully.');
        })
        .catch(err => {
            console.error('Error loading map:', err);
        });
}

// Endpoint to get status
app.get('/GET-STATUS', (req, res) => {
    if (!obstacleMap) {
        return res.status(500).send('Map not loaded');
    }

    // Simulate obstacle detection on the left and right
    const d_l = calculateDistance(robot.x, robot.y, robot.theta + Math.PI / 2);
    const d_r = calculateDistance(robot.x, robot.y, robot.theta - Math.PI / 2);
    robot.d_l = d_l;
    robot.d_r = d_r;

    // Simulated data response
    const data = {
        x: robot.x,
        y: robot.y,
        th: robot.theta,
        d_l: robot.d_l,
        d_r: robot.d_r,
        l: move.l,
        r: move.r,
    };

    res.json(data);
});

// Endpoint to move the robot
app.get('/MOVE', (req, res) => {
    const dist = parseFloat(req.query.dist) || 0;
    const steer = parseFloat(req.query.steer) || 0;

    // Update robot's position and orientation based on movement parameters
    moveRobot(dist, steer);

    // Respond with the updated position
    res.json({
        message: 'Move successful',
        x: robot.x,
        y: robot.y,
        th: robot.theta,
        d_l: robot.d_l,
        d_r: robot.d_r,
        l: move.l,
        r: move.r,
    });
});
// Endpoint to move the robot
app.get('/cMOVE', (req, res) => {
    move.l = parseFloat(req.query.l) || 0;
    move.r = parseFloat(req.query.r) || 0;

    const gm = getMove(); // [ dist, steer ]

    // Update robot's position and orientation based on movement parameters
    moveRobot(gm[0], gm[1]);

    // Respond with the updated position
    res.json({
        message: 'cMove successful',
        x: robot.x,
        y: robot.y,
        th: robot.theta,
        d_l: robot.d_l,
        d_r: robot.d_r,
        l: move.l,
        r: move.r,
    });
});
// Endpoint to move the robot
app.get('/STOP', (req, res) => {
    move.l = 0;
    move.r = 0;

    // Respond with the updated position
    res.json({
        message: 'Move successful',
        x: robot.x,
        y: robot.y,
        th: robot.theta,
        d_l: robot.d_l,
        d_r: robot.d_r,
        l: move.l,
        r: move.r,
    });
});

// Calculate distance to the nearest obstacle
function calculateDistance(x, y, theta) {
    const scale = 1;
    let d = 0;
    let prec = 1000;

    for (let li=0, lj=0 ; d<720*prec; d+=1) {
        const i = Math.round(y + Math.sin(theta)*d/prec);
        const j = Math.round(x + Math.cos(theta)*d/prec);

        if (li===i && lj===j) continue;

        const pixelColor = Jimp.intToRGBA(obstacleMap.getPixelColor(j, i));
        const bval = (pixelColor.r + pixelColor.b + pixelColor.g)/3;

        if (bval < 10) break;

        li = i; lj = j;
    }
    if(d>=720*prec) console.log('>>> calculateDistance: overflow');

    return d*scale/prec;
}



// Move the robot based on distance and steering parameters
function moveRobot(dist, steer) {
    // Update orientation
    robot.theta += steer;
    //robot.theta = robot.theta - Math.floor(robot.theta/Math.PI -1 )*Math.PI;

    // Update position based on orientation and distance
    robot.x += dist * Math.cos(robot.theta);
    robot.y += dist * Math.sin(robot.theta);
}

function cMoveRobot() {
    const gm = getMove();
    moveRobot(gm[0], gm[1]);

    console.log('Function cMoveRobot executed at', new Date(), 'xy ::', robot.x,robot.y);
}

function echoState() {
    // Simulate obstacle detection on the left and right
    const d_l = calculateDistance(robot.x, robot.y, robot.theta + Math.PI / 2);
    const d_r = calculateDistance(robot.x, robot.y, robot.theta - Math.PI / 2);
    robot.d_l = d_l;
    robot.d_r = d_r;
    
    console.log('move:', move, 'robot:', robot);
}


// Serve static files (e.g., for the web page)
app.use(express.static('public'));

// Load the map and start the server
loadMap().then(() => {
    app.listen(port, () => {
        console.log(`Server running at http://localhost:${port}`);



        console.log('map choosen: w-h', obstacleMap.bitmap.width, obstacleMap.bitmap.height);
        setInterval(cMoveRobot, 1000);
        setInterval(echoState, 7000);
    });
});
