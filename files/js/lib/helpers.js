function degreesToRadians(degrees) {
  return degrees * Math.PI / 180;
}

function isPowerOf2(value) {
  return (value & (value - 1)) === 0;
}

async function loadNetworkResourceAsText(resource){
  const response = await fetch(resource);
  const asText = await response.text();
  return asText;
}