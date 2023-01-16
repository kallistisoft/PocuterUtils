// current image file object reference
let imageFile = null;

let e_image_md5sum = null;
let e_image_appid = null;
let e_image_path = null;
let e_image_size = null;

let e_dropzone = null;
let e_drop_prompt = null;
let e_drop_info = null;
let e_button = null;
let e_progress = null;

let is_uploading = false;

function waiting( state ) {
    console.log(`waiting(${state})`);
    if( state ) {
        document.body.style.cursor =
        e_dropzone.style.cursor = 
        e_button.style.cursor = 'wait';
    } else {
        document.body.style.cursor =
        e_dropzone.style.cursor = 
        e_button.style.cursor = null;
    }
}

function UploadFile() {

    // test: not currently uploading a file
    if( is_uploading ) {
        console.error("UploadFile() - called while in use!");
        return;
    }

    // verify: image file exists
    if( imageFile == null ) {
        waiting( false );

        console.error("UploadFile() - called with non-existant imageFile!");
        return;
    }

    // verify: image file size is reasonable
    if( imageFile.size < 600*1024 ) {
        imageFile == null;
        waiting( false );

        alert("Error image file must be >= 600KiB in size!");
        return;
    }

    // set: uploading state busy flags
    console.log(`Upload: ${imageFile.name}`);
    is_uploading = true;
    waiting( true );

    // update: display element text
    e_image_appid.innerText = imageFile.appid;
    e_image_path.innerText = './' + imageFile.fullpath;
    e_image_size.innerText = `${(imageFile.size / 1024).toFixed(2)} Kib`;
    e_image_md5sum.innerText = imageFile.md5sum;

    // update: display element attributes
    e_dropzone.setAttribute('loaded','true');    
    e_button.setAttribute('enabled','true');
    e_button.style.display = 'none';
    e_progress.style.display = 'block';

    // create: parameters object
    const params = new FormData();    
    params.append( 'appID', imageFile.appid );
    params.append( 'appMD5', imageFile.md5sum );    
    params.append( 'appSize', imageFile.size );
    params.append( 'appImage', imageFile );

    // create: xhr request object
    const xreq = new XMLHttpRequest();

    // onprogress: update progress bar
    xreq.upload.onprogress = ( event ) => {
        if (event.lengthComputable) {
            const percentage = Math.round((event.loaded * 100) / event.total);
            e_progress.value = percentage;
            console.log('progress: ',percentage);
        }
    }

    // onloadend: clear wait state flags and 
    xreq.onloadend = () => {
        if( xreq.status === 200 ) {
            console.log( xreq.responseText );            
            console.log('Done uploading file...');
            alert( xreq.responseText );
        }
        waiting( false );
        is_uploading = false;
        e_progress.value = 0;
        e_button.style.display = 'block';
        e_progress.style.display = 'none';
    };

    // Initiate a multipart/form-data upload
    xreq.open( "POST", '/upload', true );
    xreq.send( params );
}

window.onload = () => {
    console.log("window.onload()");

    e_dropzone = document.getElementById('dropzone');  
    e_drop_prompt = document.getElementById('drop_prompt');
    e_drop_info = document.getElementById('drop_info');

    e_image_md5sum = document.getElementById('image_md5sum');
    e_image_appid = document.getElementById('image_appid');
    e_image_path = document.getElementById('image_path');
    e_image_size = document.getElementById('image_size');

    e_button = document.getElementById('button');

    e_progress = document.getElementById('progress_bar');

    window.addEventListener('drop', (event) => { 
        event.preventDefault(); 
    });

    window.addEventListener('dragover', (event) => { 
        event.dataTransfer.dropEffect = 'none';
        event.preventDefault(); 
    });

    // set: display dragover attribute
    e_dropzone.ondragenter = e_dropzone.ondragover = (event) => { 
        event.stopPropagation();
        event.preventDefault();
        if( !event.target == e_dropzone ) return;
        if( e_dropzone.getAttribute('loaded') === 'true' ) {
            e_dropzone.setAttribute('dragover','true');
        }
    }

    // reset: display dragover attribute
    document.body.ondragenter = (event) => { 
        event.stopPropagation();
        event.preventDefault();
        if( !event.target == e_dropzone ) return;
        if( e_dropzone.getAttribute('loaded') === 'true' ) {
            e_dropzone.setAttribute('dragover','false');
        }
    }

    e_dropzone.ondrop = (event) => {

        // clear: drag event state
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        event.stopPropagation();
        event.preventDefault();

        // set: element CSS state attributes
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        e_dropzone.setAttribute('dragover','false');
        e_dropzone.setAttribute('loaded','false');
        e_button.setAttribute('enabled','false');        
        
        // reset imageFile and control states
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        imageFile = null;
        e_image_md5sum.innerText = "";        
        e_image_appid.innerText = "";
        e_image_path.innerText = "";
        e_image_size.innerText = "";

        // test that only one folder was dropped
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        const filesArray = event.dataTransfer.files;
        if( filesArray.length > 1 ) {
            alert("Only one file may be dragged and dropped at a time!");
            return;
        }
        
        waiting(true);

        // traverseFileTree(item, path = "") :: recursively walk file tree
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // TODO: limit depth recursion to five folders, add relevant error message
        let is_traversing = false;
        function traverseFileTree(item, path = "", depth = 0 ) {
            path = path || "";
            
            if( depth >= 5 ) {
                console.error(`traverseFileTree(...) ${depth} limit reached`);                
                return;
            }

            // exit: already found image file
            if( imageFile != null ) return;

            // file: parse full path name and check for appid folder
            if (item.isFile) {
                item.file(function(file) {
                    let appid = parseInt( path.split('/').reverse()[1] );
                    if( isNaN( appid ) || appid < 2 ) return;
                    if( file.name === 'esp32c3.app' ) {
                        imageFile = file;
                        imageFile.fullpath = path + file.name;
                        imageFile.appid = appid;
                        console.log("id:", imageFile.appid );
                        console.log("path:", imageFile.fullpath );
                    }
                });
            } 

            // dir: iterate sub-folders and recurse
            else if (item.isDirectory) {
                var dirReader = item.createReader();
                dirReader.readEntries(function(entries) {
                    for (var i=0; i<entries.length; i++) {
                        traverseFileTree(entries[i], path + item.name + "/", depth + 1 );
                    }
                });
            }

            if( !depth ) is_traversing = false;
        }

        // get transfer item and walk file tree
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        var items = event.dataTransfer.items;
        var item = items[0].webkitGetAsEntry();
        if (item) {
            is_traversing = true;
            traverseFileTree( item );
        }

        // ugly hack to get around async behavior of directory transversal
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        let timer = window.setInterval( () => {

            if( is_traversing ) return;
            window.clearInterval( timer );

            if( imageFile == null ) {
                alert("Could not find Pocuter image file 'esp32c3.app'!");
                waiting(false);
                return;
            }

            var reader = new FileReader();            
            reader.onload = function(event) {
                var wordArray = CryptoJS.lib.WordArray.create(this.result);
                var md5 = CryptoJS.MD5(wordArray).toString();
                imageFile.md5sum = md5;

                console.log('md5:', imageFile.md5sum );
                console.log('size:', imageFile.size );

                UploadFile();
            };
            reader.readAsArrayBuffer( imageFile );
        }, 200 );
    };
}