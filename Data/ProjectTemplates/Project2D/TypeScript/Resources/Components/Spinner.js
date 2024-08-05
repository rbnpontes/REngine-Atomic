"atomic component";
"use strict";
var __extends = (this && this.__extends) || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
};
/**
 * Component that will rotate a node at a configurable speed.
 */
var Star = (function (_super) {
    __extends(Star, _super);
    function Star() {
        _super.apply(this, arguments);
        /**
         * Fields witihin the inspectorFields object will be exposed to the editor
         */
        this.inspectorFields = {
            speed: 1.0
        };
    }
    /**
     * Update called every cycle with timeStep containing the delta between calls
     * @param  {number} timeStep time since last call to update
     */
    Star.prototype.update = function (timeStep) {
        this.node.rotate2D(timeStep * 75 * this.speed);
    };
    return Star;
})(Atomic.JSComponent);
exports.default = Star;
