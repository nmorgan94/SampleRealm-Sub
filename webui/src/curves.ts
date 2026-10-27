// Ports of the C++ maths, checked against the engine in test/curves.test.ts

export const shapeProgress = (progress: number, tension: number) => {
  const minimumTension = 0.001;
  const maximumSteepness = 20;

  if (progress <= 0 || progress >= 1 || Math.abs(tension) < minimumTension) {
    return Math.min(1, Math.max(0, progress));
  }

  const steepness = 1 + Math.abs(tension) * (maximumSteepness - 1);
  return tension > 0
    ? Math.log(1 + progress * (steepness - 1)) / Math.log(steepness)
    : (Math.pow(steepness, progress) - 1) / (steepness - 1);
};

export const driveEnvelopeModulation = (env: number, amount: number) => {
  const departure = Math.max(0, 1 - env);
  return amount >= 0 ? 1 - amount * departure : 1 + amount * (1 - Math.sqrt(departure));
};

export const timeToWidthFraction = (seconds: number) => {
  const minTime = 0.001;
  const maxTime = 5;
  const steepness = 50;
  const clamped = Math.min(maxTime, Math.max(minTime, seconds));
  return Math.log1p(steepness * (clamped - minTime)) / Math.log1p(steepness * (maxTime - minTime));
};
