//
//  Created by Stanislav Ratashnyuk on 30/04/2024.
//

import AVFoundation
import VideoToolbox
import AppKit

struct PassToDecompressCallback {
    var width: UInt32 = 0
    var height: UInt32 = 0
    var timecode: Int64 = 0
    var counter: Int = 1
    var dngProxy: DngBridge
    var outputFolder: String
    var exifInfo: PassToExif
}

var drp_test = dng_request_params();
drp_test.fileName = "123"


struct PassToExif {
    var manufacurer: String = ""
    var modelName: String = ""
    var fNumber: String = ""
    var irisNumber: String = ""
    var shutterSpeed: String = ""
    var shutterAngle: String = ""
    var whiteBalance: String = ""
    var iso: String = ""
    var lensModel: String = ""
    var lensAttributes: String = ""
}

class Decoder {
    private let assetReader: AVAssetReader?
    private let output: AVAssetReaderTrackOutput
    private var decompressionSession: VTDecompressionSession!
    private var passToExif: PassToExif
    private var overallDuration: CMTime
    
    init(inputFile: String, outputFolder: String) throws {
        let movieUrl = URL(fileURLWithPath: inputFile)
        let movieAsset = AVAsset(url: movieUrl)
            
        passToExif = PassToExif()
                
        do {
            let assetReader = try AVAssetReader(asset: movieAsset)
            let tracks = movieAsset.tracks(withMediaType: .video)
            
            guard let firstTrack = tracks.first else {
                print("No video tracks found")
                throw NSError()
            }
            
            overallDuration = firstTrack.timeRange.duration;
            
            let out = AVAssetReaderTrackOutput(track: firstTrack, outputSettings: nil)
            out.alwaysCopiesSampleData = true
            
            assetReader.add(out)
            
            self.assetReader = assetReader
            self.output = out
            
            movieAsset.loadMetadata(for: AVMetadataFormat.quickTimeMetadata, completionHandler: { avmd, _ in
                avmd?.map({ AVMetadataItem in
                    switch(AVMetadataItem.identifier?.rawValue) {
                    case "mdta/com.apple.proapps.manufacturer":
                        self.passToExif.manufacurer = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/com.apple.proapps.modelname":
                        self.passToExif.modelName = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/com.apple.proapps.exif.%7BExif%7D.FNumber":
                        self.passToExif.fNumber = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/org.smpte.rdd18.lens.irisfnumber":
                        self.passToExif.irisNumber = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/com.apple.proapps.exif.%7BExif%7D.ShutterSpeedValue":
                        self.passToExif.shutterSpeed = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/org.smpte.rdd18.camera.shutterspeed_angle":
                        self.passToExif.shutterAngle = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/org.smpte.rdd18.camera.whitebalance":
                        self.passToExif.whiteBalance = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/org.smpte.rdd18.camera.isosensitivity":
                        self.passToExif.iso = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/com.apple.proapps.exif.%7BExifAux%7D.LensModel":
                        self.passToExif.lensModel = AVMetadataItem.stringValue ?? ""
                        break
                    case "mdta/org.smpte.rdd18.lens.lensattributes":
                        self.passToExif.lensAttributes = AVMetadataItem.stringValue ?? ""
                        break
                    default:
                        break
                    }
                })
                
            })
            
        } catch {
            print(error)
            throw error
        }
        
    }
    
    func run(){
        guard let assetReader = assetReader, assetReader.startReading() else {
            print("Failed to stard asset reader")
            return
        }
        
        
        var dngProxy = DngBridge()
        var counter = 1
        
        print("Start converting...")
        
        while(assetReader.status == .reading) {
            guard let sampleBuffer = output.copyNextSampleBuffer() else {
                //print(assetReader.status.rawValue)
                //print(assetReader.error as Any)
                continue
            }
                        
            //print("Decoding success! \(sampleBuffer)")
            
            
            var vd = PassToDecompressCallback(dngProxy: dngProxy, outputFolder: outputFolder!, exifInfo: passToExif)
            
            var timecode = CMSampleBufferGetDecodeTimeStamp(sampleBuffer)
            
            //var percentage = timecode.value  / overallDuration.value * 100;
                        
            print("frame timestamp \(timecode.value) of \(overallDuration.value)");
            
            if (sampleBuffer.formatDescription != nil) {
                vd.height = UInt32(sampleBuffer.formatDescription!.dimensions.height)
                vd.width = UInt32(sampleBuffer.formatDescription!.dimensions.width)
                vd.timecode = timecode.value
                vd.counter = counter
            }
            
            counter += 1
            
            if let formatDescription = CMSampleBufferGetFormatDescription(sampleBuffer) {
                
                
                if decompressionSession == nil {
                    // print("format: \(String(describing: sampleBuffer.formatDescription?.parameterSets))")
                    
                    let imageBufferAttributes: [CFString: Any] = [
                        kCVPixelBufferPixelFormatTypeKey: kCVPixelFormatType_16VersatileBayer
                    ]
                    
                    let decoderSpecification: [CFString: Any] = [
                        kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder: false
                    ]
                    
                    let vTDecompressionCallback: VTDecompressionOutputCallback = { vd, _, status, infoFlags, imageBuffer, presentationTimeStamp, presentationDuration in
                        
                        assert(noErr == status)
                        
                        //print("decode callback status: \(status), bayer imageBuffer \(String(describing: imageBuffer)), flags: \(infoFlags), pts: \(presentationTimeStamp), ptsd: \(presentationDuration)")
                        
                        var passToDecompressCallback = vd!.load(as: PassToDecompressCallback.self)
                        
                        CVPixelBufferLockBaseAddress(imageBuffer!, CVPixelBufferLockFlags.readOnly)
                        
                        var iBP = CVPixelBufferGetBaseAddress(imageBuffer!)
                        
                        var iPBSize = CVPixelBufferGetDataSize(imageBuffer!);
                        
                        /*
                        let blackLevel = imageBuffer!.attachments.propagated["ProResRAW_BlackLevel"]
                        let whiteLevel = imageBuffer!.attachments.propagated["ProResRAW_WhiteLevel"]
                        let width = videoDimensions.width
                        let heigth = videoDimensions.heigth
                        let timecode = videoDimensions.timecode
                        */
                        
                        var cfd = CVBufferGetAttachments(imageBuffer!, CVAttachmentMode.shouldPropagate)! as! [String: AnyObject]
                        let blackLevel = UInt32(cfd["ProResRAW_BlackLevel"] as! NSNumber)
                        let whiteLevel = UInt32(cfd["ProResRAW_WhiteLevel"] as! NSNumber)
                        
                        let manufacturer = std.string(passToDecompressCallback.exifInfo.manufacurer)
                        let model = std.string(passToDecompressCallback.exifInfo.modelName)
                        let fNumber = std.string(passToDecompressCallback.exifInfo.fNumber)
                        let fIrisNumber = fNumber
                        let shutterSpeed = std.string(passToDecompressCallback.exifInfo.shutterSpeed)
                        let shutterAngle = std.string(passToDecompressCallback.exifInfo.shutterAngle)
                        let lensMaker = std.string(passToDecompressCallback.exifInfo.lensModel)
                        let lensAttributes = std.string(passToDecompressCallback.exifInfo.lensAttributes)
                        let iso = std.string(passToDecompressCallback.exifInfo.iso)
                        let width = passToDecompressCallback.width
                        let heigth = passToDecompressCallback.height
                        
                        //let timecode = passToDecompressCallback.timecode
                        let fileName = "\(passToDecompressCallback.outputFolder)image_\(passToDecompressCallback.counter).dng"
                        
                        //if (passToDecompressCallback.counter <= 1690) {
                        //    return
                        //}
                         
                        passToDecompressCallback.dngProxy.request_dng(iBP!, Int32(iPBSize), std.string(fileName), manufacturer, model, fNumber, fIrisNumber, shutterSpeed, shutterAngle, iso, lensMaker, lensAttributes, width, heigth, blackLevel, whiteLevel)
                    }
                    
                    var outputCallback = VTDecompressionOutputCallbackRecord(decompressionOutputCallback: vTDecompressionCallback, decompressionOutputRefCon: &vd)
                    
                    let status = VTDecompressionSessionCreate(allocator: nil, formatDescription: formatDescription, decoderSpecification: decoderSpecification as CFDictionary, imageBufferAttributes: imageBufferAttributes as CFDictionary, outputCallback: &outputCallback, decompressionSessionOut: &decompressionSession)
                    
                    assert(noErr == status)
                }

                let status = VTDecompressionSessionDecodeFrame(decompressionSession, sampleBuffer: sampleBuffer, flags: [], frameRefcon: nil, infoFlagsOut: nil)
                assert(noErr == status)
            }
        }
        
        dngProxy.wait_until_complete()
    }
}

var inputFile = UserDefaults.standard.string(forKey: "i")
var outputFolder = UserDefaults.standard.string(forKey: "o")

if (inputFile == nil) {
    print("No input file")
    exit(1)
}

if (outputFolder == nil) {
    print("No output folder")
    exit(1)
}

var c = try Decoder(inputFile: inputFile!, outputFolder: outputFolder!)
c.run()
