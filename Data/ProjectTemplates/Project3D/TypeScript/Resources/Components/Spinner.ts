"atomic component";

/**
 * Component that will rotate a node at a configurable speed.
 */
export default class Spinner extends Atomic.JSComponent {

    /**
     * Fields witihin the inspectorFields object will be exposed to the editor
     */
    inspectorFields = {
        speed: 1.0
    };

    /**
     * The speed in which to turn the component
     * @type {number}
     */
    speed: number;

    /**
     * Update called every cycle with timeStep containing the delta between calls
     * @param  {number} timeStep time since last call to update
     */
    update(timeStep) {

        this.node.yaw(timeStep * 75 * this.speed);

    }
}
