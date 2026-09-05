import { Viewer } from './viewer.js';

/**
 * Initialize the Graph Editor.
 */
export function init() {
  const viewer = new Viewer();
  viewer.loadEnumeratedValues();
  document.body.appendChild(viewer);
}