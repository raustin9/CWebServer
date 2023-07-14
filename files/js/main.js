import RenderEngine from './RenderEngine.js'

// GET CANVAS CONTEXT TO PASS TO THE ENGINE
// THE ENGINE WILL CREATE THE GL CONTEXT AND HENDLE RENDERING
let canvas = document.getElementById("main-canvas");
const engine = new RenderEngine(canvas);
engine.SetLightPosition([20, 2, 5])

let fighterMap = {
  name: 'fighter',
  vertexSource: 'resources/shaders/vertex/texture.vert',
  fragmentSource: 'resources/shaders/fragment/texture.frag',
  objectSource: 'resources/models/fighter.obj',
  textureSource: 'resources/textures/fighter.jpg'
};

let cruiserMap = {
  name: 'cruiser',
  vertexSource: 'resources/shaders/vertex/normal.vert',
  fragmentSource: 'resources/shaders/fragment/normal.frag',
  objectSource: 'resources/models/USSEnterprise.obj',
  textureSource: 'resources/textures/enterprise.png',
  normalSource: 'resources/textures/bump6.jpg'
};

let planetMap = {
  name: 'planet',
  vertexSource: 'resources/shaders/vertex/normal.vert',
  fragmentSource: 'resources/shaders/fragment/normal.frag',
  objectSource: 'resources/models/sphereT.obj',
  textureSource: 'resources/textures/Terrestrial1.png',  
  normalSource: 'resources/textures/bump5.jpg'
}

let sunMap = {
  name: 'sun',
  vertexSource: 'resources/shaders/vertex/sun.vert',
  fragmentSource: 'resources/shaders/fragment/sun.frag',
  objectSource: 'resources/models/sphereT.obj',
  textureSource: 'resources/textures/sun.jpg',
}

let portalMap = {
  name: "portal",
  vertexSource: 'resources/shaders/vertex/env.vert',
  fragmentSource: 'resources/shaders/fragment/env.frag',
  objectSource: 'resources/models/Crystal.obj',
  environmentSource: [
    'resources/textures/cubemap/portal.jpg',
    'resources/textures/cubemap/portal.jpg',
    'resources/textures/cubemap/portal.jpg',
    'resources/textures/cubemap/portal.jpg',
    'resources/textures/cubemap/portal.jpg',
    'resources/textures/cubemap/portal.jpg',
  ]
};

let skyMap = {
  vertexSource: 'resources/shaders/vertex/skybox.vert',
  fragmentSource: 'resources/shaders/fragment/skybox.frag',
  environmentSource: [
    // 'resources/textures/cubemap/posx.jpg',
    // 'resources/textures/cubemap/negx.jpg',
    // 'resources/textures/cubemap/posy.jpg',
    // 'resources/textures/cubemap/negy.jpg',
    // 'resources/textures/cubemap/posz.jpg',
    // 'resources/textures/cubemap/negz.jpg',
    'resources/textures/cubemap/lightblue/right.png',
    'resources/textures/cubemap/lightblue/left.png',
    'resources/textures/cubemap/lightblue/bot.png',
    'resources/textures/cubemap/lightblue/top.png',
    'resources/textures/cubemap/lightblue/front.png',
    'resources/textures/cubemap/lightblue/back.png',
  ]
};

engine.CreateSkybox(
  skyMap
).then(() => {

})

// CREATE FIGHTERS
engine.CreateObject(
  fighterMap
).then(() => {
  engine.SetObjectAnimate("fighter", () => {
    engine.Translate("fighter", [0, -0.3, -5]);
    engine.Rotate("fighter", [1, 0, 0], 3 * Math.PI / 2);
    engine.Rotate("fighter", [0, 0, 1], Math.PI);
    engine.Rotate("fighter", [1, 0, 0], -0.2);
  });
});

engine.CreateObject(
  fighterMap,
  "fighter2"
).then(() => {
  engine.SetObjectAnimate("fighter2", () => {
    engine.Translate("fighter2", [1.5, -1.3, -3.7]);
    engine.Rotate("fighter2", [1, 0, 0], 3 * Math.PI / 2);
    engine.Rotate("fighter2", [0, 0, 1], Math.PI);
    engine.Rotate("fighter2", [1, 0, 0], -0.15);
  });
});

engine.CreateObject(
  fighterMap,
  'fighter3'
).then(() => {
  engine.SetObjectAnimate("fighter3", () => {
    engine.Translate("fighter3", [-1.5, -1.3, -3.7]);
    engine.Rotate("fighter3", [1, 0, 0], 3 * Math.PI / 2);
    engine.Rotate("fighter3", [0, 0, 1], Math.PI);
    engine.Rotate("fighter3", [1, 0, 0], -0.15);
  });
});

// CREATE THE USS ENTERPRISE
engine.CreateObject(
  cruiserMap
).then(() => {
  engine.SetObjectAnimate("cruiser", () => {
    engine.Translate("cruiser", [5, 0, -30]);
    
    engine.Scale("cruiser", [10, 10, 10]);
    engine.Rotate("cruiser", [0, 0, 1], Math.PI / 6);
    engine.Rotate("cruiser", [0, 1, 0], -Math.PI / 8);
    engine.Rotate("cruiser", [1, 0, 0], Math.PI / 16);

    
  })
})

// CREATE THE GIANT PLANET IN THE BACKGROUND
engine.CreateObject(
  planetMap
).then(() => {
  engine.SetObjectAnimate("planet", () => {
    engine.Translate("planet", [-200, 500, -5000]);
    engine.Scale("planet", [3000, 3000, 3000]);
    engine.Rotate("planet", [1, 0.5, 0], Math.PI / 4);
    engine.Spin("planet", [0, 1, 0], 0.1);
  })
});

// CREATE THE SUN IN THE DISTANCE
engine.CreateObject(
  sunMap
).then(() => {
  engine.SetObjectAnimate("sun", () => {
    engine.Translate("sun", [8000, 4000, -4000]);
    engine.Scale("sun", [1000, 1000, 1000]);
  })
})

// // CREATE THE PORTALS
// for (let i = 0; i < 6; i++) {
//   engine.CreateObject(
//     portalMap
//     ,`portal${i}`
//   ).then(() => {
//     engine.SetObjectAnimate(`portal${i}`, () => {
//       engine.Translate(`portal${i}`, [-100 + i * 40, 20, -100]);
//       engine.Revolve(`portal${i}`, [6, 3, -5], 0.5);
//       engine.Rotate(`portal${i}`, [0, 1, 0], Math.PI / 2);
//       engine.Scale(`portal${i}`, [8, 15, 8]);
//       engine.Spin(`portal${i}`, [0, 1, 0], 0.2);
      
//     })
//   })
// }