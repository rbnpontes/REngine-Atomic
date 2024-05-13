// Rewrite PackageTool to NodeJS
// This is used to bundle iOS and Android Resource Assets

const fs = require('fs');
const path = require('path');
const glob = require('glob');
const { argv, off } = require('process');

const ignore_extensions = ['.bak', '.rule'];
const pkg_file_id = 'RPAK';
const header_size = [
    4, // 4 bytes file id
    4, // 4 bytes items count
    4, // 4 bytes package checksum
].reduce((prev, curr)=> prev + curr);
/**
 * 
 * @param {number} hash 
 * @param {number} c 
 * @returns {number}
 */
function sdbm_hash(hash, c) {
    return c + (hash << 6) + (hash << 16) - hash;
}
/**
 * 
 * @param {Buffer} buffer 
 * @param {number} length
 * @param {number} offset
 */
function hash_buffer(buffer, length, offset) {
    let checksum = 0;
    for(let i = 0; i < length; ++i)
        checksum = sdbm_hash(checksum, buffer.readUInt8(offset + i));
    return checksum >>> 0;
}

/**
 * Write string into buffer
 * @param {string} input 
 * @param {Buffer} buffer 
 * @param {number} offset 
 */
function write_string(input, buffer, offset) {
    for(let i = 0; i < input.length; ++i)
        offset = buffer.writeUint8(input.charCodeAt(i), offset);
    // write null terminator
    return buffer.writeUint8(0, offset);
}

/**
 * 
 * @param {string} directoryPath 
 */
function mkdir_recursive(directoryPath) {
    directoryPath = path.resolve(directoryPath);
    const parts = directoryPath.split(path.sep);
    let dir = parts[0];
    for(let i = 1; i < parts.length; ++i) {
        dir = path.join(dir, parts[i]);
        if(fs.existsSync(dir))
            continue;
        fs.mkdirSync(dir);
    }
}
/**
 * Read string from buffer
 * @param {Buffer} buffer 
 * @param {number} offset 
 * @returns {string}
 */
function read_string(buffer, offset) {
    let str = '';
    while(offset < buffer.byteLength) {
        const char = buffer.readUint8(offset);
        if(char == 0)
            break;
        str += String.fromCharCode(char);
        ++offset;
    }

    return str;
}

class ResourceEntry {
    constructor() {
        this.fullpath = '';
        this.name = '';
        this.checksum = 0;
        this.offset = 0;
        this.size = 0;
    }

    getBuffer() {
        return fs.readFileSync(this.fullpath);
    }

    getDataSize() {
        return [
            this.name.length + 1, // name in bytes + null terminator
            4, // 4 bytes offset
            4, // 4 bytes size
            4, // 4 bytes checksum
        ].reduce((prev, curr)=> prev + curr);
    }
}

class PackageHeader {
    constructor() {
        this.fileId = '';
        this.size = 0;
        this.checksum = 0;
    }
}

/**
 * @type {Array<ResourceEntry>}
 */
const resource_entries = [];
class PackageTool {
    /**
     * @param {string} directory 
     * @param {string} outputPath 
     * @param {boolean} isBigEndian
     */
    constructor(directory, outputPath, isBigEndian) {
        this.directory = directory;
        this.outputPath = outputPath;
        this.checksum = 0;
        this.entriesHeaderSize = 0; // helper var used to estimate entries size
        this.size = header_size;
        this.isBigEndian = isBigEndian;
    }
    /**
     * Collect Files
     */
    collect() {
        const files = glob.globSync('**/*', { cwd : this.directory });
        let offset = 0;
        files.forEach(x => {
            const must_ignore = Boolean(ignore_extensions.find(ext => x.endsWith(ext)));
            const fullPath = path.join(this.directory, x);
            const stat = fs.statSync(fullPath);
            
            if(must_ignore || stat.isDirectory())
                return;

            const entry = new ResourceEntry();
            entry.name = x;
            entry.fullpath = fullPath;
            entry.size = stat.size;
            entry.offset = offset;

            // calculate offsets and package total size
            offset += entry.size;
            this.entriesHeaderSize += entry.getDataSize();
            this.size += entry.size + entry.getDataSize();

            resource_entries.push(entry);
        });
    }

    calculateOffsets() {
        const baseResourceDataOffset = header_size + this.entriesHeaderSize;
        resource_entries.forEach(entry => {
            entry.offset += baseResourceDataOffset;
        });
    }

    calculateChecksum() {
        resource_entries.forEach(entry => {
            const buffer = entry.getBuffer();
            for(let i = 0; i < buffer.byteLength; ++i) {
                const value = buffer.readUint8(i);
                this.checksum = sdbm_hash(this.checksum, value);
                entry.checksum = sdbm_hash(entry.checksum, value);
            }

            entry.checksum = entry.checksum >>> 0;
        });
    }

    /**
     * @returns package output directory
     */
    build() {
        const buffer = Buffer.alloc(this.size);

        let offset = this.buildHeader(buffer, 0);
        
        if(offset != header_size) {
            console.log(`Package Header Size: ${header_size} | Offset: ${offset}`);
            throw new Error('Something is wrong. Offset is not equal to header size');
        }

        resource_entries.forEach(entry => {
            offset = this.processEntryHeader(entry, buffer, offset);
        });

        if(this.entriesHeaderSize != (offset - header_size)) {
            console.log(`Entries Header Size: ${this.entriesHeaderSize} | Offset: ${(offset - header_size)}`);
            throw new Error("Something is wrong. It seems that entries header write exceeds the expected size.");
        }
    
        resource_entries.forEach(entry => {
            offset = this.processEntry(entry, buffer, offset);
        });

        if(this.size != offset)
            throw new Error("Something is wrong. It seems that write package exceeds the expected size.");

        const directoryPath = path.dirname(this.outputPath);
        mkdir_recursive(directoryPath);

        if(fs.existsSync(this.outputPath))
            fs.unlinkSync(this.outputPath);
        fs.writeFileSync(this.outputPath, buffer);
        return this.outputPath;
    }

    /**
     * Build header
     * @param {Buffer} buffer 
     * @param {number} offset
     * @returns {number} new buffer offset
     */
    buildHeader(buffer, offset) {
        // force checksum to be uint32
        this.checksum = this.checksum >>> 0;
        // Write file id. Note: File Id doesn't contains null terminator
        for(let i =0; i < pkg_file_id.length; ++i)
            offset = buffer.writeUint8(pkg_file_id.charCodeAt(i), offset);

        const write_call = (this.isBigEndian ? buffer.writeUInt32BE : buffer.writeUint32LE).bind(buffer);
        // write package size
        offset = write_call(resource_entries.length, offset);
        // write package checksum
        return write_call(this.checksum, offset);
    }

    /**
     * Write Resource Entries on Buffer
     * @param {ResourceEntry} entry 
     * @param {Buffer} buffer 
     * @param {number} offset 
     */
    processEntryHeader(entry, buffer, offset) {
        // force checksum to be uint32
        const checksum = entry.checksum >>> 0;
        offset = write_string(entry.name, buffer, offset);

        const write_call = (this.isBigEndian ? buffer.writeUInt32BE : buffer.writeUInt32LE).bind(buffer);
        offset = write_call(entry.offset, offset);
        offset = write_call(entry.size, offset);
        return write_call(checksum, offset);
    }

    /**
     * 
     * @param {ResourceEntry} entry 
     * @param {Buffer} buffer 
     * @param {number} offset 
     */
    processEntry(entry, buffer, offset) {
        const file_buffer = entry.getBuffer();
        return offset + file_buffer.copy(buffer, offset, 0);
    }

    /**
     * Validate package file
     * @param {string} packagePath 
     */
    validate(packagePath) {
        const buffer = fs.readFileSync(packagePath);
        const header = new PackageHeader();

        if(buffer.byteLength != this.size)
            throw new Error('Package Size doesn\'t matches current package size.');

        let offset = this.readHeader(buffer, header, 0);
        if(header.size != resource_entries.length)
            throw new Error('Header Size doesn\'t matches resource entries length.');

        if(header.checksum != this.checksum) {
            console.error(`Checksum: ${this.checksum} | Package Checksum: ${header.checksum}`);
            throw new Error('Package file is corrupted. Package checksum doesn\'t matches.');
        }

        /**
         * @type {Array<ResourceEntry>}
         */
        const entries = new Array(header.size);
        for(let i =0; i < entries.length; ++i) {
            const entry = new ResourceEntry();
            entries[i] = entry;

            offset = this.readEntryHeader(buffer, entry, offset);
        }

        // validate entries.
        entries.forEach((entry, idx)=> {
            if(entry.name !== resource_entries[idx].name)
                throw new Error('Invalid package entry. package entry name doesn\'t matches.');
            if(entry.size != resource_entries[idx].size)
                throw new Error('Invalid package entry. package entry size doesn\'t matches.');
            if(entry.checksum != resource_entries[idx].checksum){
                console.log(resource_entries[idx], entry);
                throw new Error('Invalid package entry. package entry checksum doesn\'t matches.');
            }

            // calculate original data checksum.
            const dataChecksum = hash_buffer(buffer, entry.size, entry.offset);
            if(dataChecksum != entry.checksum) {
                console.log(entry);
                throw new Error('Invalid package entry data. it seems offset is wrong or data is corrupted.');
            }
        });
    }

    /**
     * 
     * @param {Buffer} buffer 
     * @param {PackageHeader} header 
     * @param {number} offset
     * @returns new buffer offset 
     */
    readHeader(buffer, header, offset) {
        if(buffer.byteLength < header_size)
            throw new Error('Invalid Package Data. Buffer size doesn\'t matches required package header size.');
        
        // read file id
        for(let i = 0; i < pkg_file_id.length; ++i) {
            const char = buffer.readUint8(offset);
            header.fileId += String.fromCharCode(char);
            ++offset;
        }

        const read_call = (this.isBigEndian ? buffer.readUInt32BE : buffer.readUint32LE).bind(buffer);
        header.size = read_call(offset);
        offset += 4;
        header.checksum = read_call(offset);
        return offset + 4;
    }

    /**
     * 
     * @param {Buffer} buffer 
     * @param {ResourceEntry} entry
     * @param {number} offset
     * @returns {number} 
     */
    readEntryHeader(buffer, entry, offset) {
        entry.name = read_string(buffer, offset);
        
        if(entry.name.length >= (buffer.length - header_size))
            throw new Error('Invalid package file. It seems resource is corrupted.');

        offset += entry.name.length + 1;

        const read_call = (this.isBigEndian ? buffer.readUint32BE : buffer.readUint32LE).bind(buffer);
        entry.offset = read_call(offset);
        offset += 4;
        entry.size = read_call(offset);
        offset += 4;
        entry.checksum = read_call(offset);
        offset += 4;
        return offset;
    }
}

function print_usage() {
    console.log([
        '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',
        '@@@ REngine - PackageTool                                     @@@',
        '@@@ Usage: yarn pkg [directory] [package-output-path].pak     @@@',
        '@@@        [0=little endian|1=big endian]                     @@@',
        '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@'
    ].join('\n'));
}
function error(err_desc) {
    console.error(`Error: ${err_desc}`);
    print_usage();
    process.exit(1);
}

function getArgs() {
    const args = [...argv];
    const selfCmdIdx = args.findIndex(x => x.endsWith('PackageTool.js'));
    const result = args.splice(selfCmdIdx + 1, args.length);
    if(result.length < 3)
        error('Invalid arguments');
    return result;
}

const [directory, pkgPath, isBigEndian] = getArgs();
{
    if(!directory)
        error('Invalid arguments. Directory is empty');
    
    if(!pkgPath)
        error('Invalid arguments. Package output path is empty');
    
    if(!pkgPath.endsWith('.pak'))
        error('Invalid arguments. Package output path must ends with extension .pak');

    if(isBigEndian != '0' && isBigEndian != '1')
        error('Invalid arguments. Endianess must be specified');
}

const pkg = new PackageTool(
    path.resolve(directory), 
    pkgPath, 
    Boolean(parseInt(isBigEndian) | 0)
);

console.log('- Collecting files');
pkg.collect();
console.log('- Calculating offsets');
pkg.calculateOffsets();
console.log('- Calculating checksum');
pkg.calculateChecksum();
console.log('- Building Package');
const packageOutputPath = pkg.build();
console.log('- Validating Package');
try {
    pkg.validate(packageOutputPath);
} catch(e) {
    console.error('- Invalid Package. Exiting!');
    console.error(e.message);
    process.exit(0);
}
console.log('- 🎉 Finished');