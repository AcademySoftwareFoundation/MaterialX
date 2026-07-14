import { DropdownHelper } from './helper.js';
import { DropHandling } from './dropHandling.js';

/**
 * The Graph Editor viewer.
 */
export class Viewer {
  /**
   * Initialize the viewer.
   */
  constructor() {
    this.select = document.createElement('select');
    this.select.id = 'enumerated-values';
    this.select.addEventListener('change', () => {
      DropHandling.init(this.select);
    });
    this.appendChild(this.select);
  }

  /**
   * Get the list of enumerated values and create dropdown menu options.
   */
  async loadEnumeratedValues() {
    const values = await DropdownHelper.getEnumeratedValues();
    values.forEach((value) => {
      const option = DropdownHelper.createOption(value);
      this.select.appendChild(option);
    });
  }
}